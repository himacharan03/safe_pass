document.getElementById('scenarioForm').addEventListener('submit', async (e) => {
    e.preventDefault();

    const data = {
        speed_a: document.getElementById('speedA').value,
        speed_b: document.getElementById('speedB').value,
        speed_c: document.getElementById('speedC').value,
        dist_ab: document.getElementById('distAB').value,
        dist_bc: document.getElementById('distBC').value,
        opp_present: document.getElementById('oppPresent').value,
        road_type: document.getElementById('roadType').value,
        visibility: document.getElementById('visibility').value
    };

    try {
        const response = await fetch('/predict', {
            method: 'POST',
            headers: { 'Content-Type': 'application/json' },
            body: JSON.stringify(data)
        });

        const result = await response.json();
        updateUI(result, data.opp_present);
        startAnimation(result.decision, data.speed_a, data.speed_b, data.speed_c, data.dist_ab, data.dist_bc, data.opp_present);
    } catch (error) {
        console.error("Error fetching simulation:", error);
    }
});

let animationInterval;

function updateUI(result, opp_present) {
    const decisionText = document.getElementById('decisionText');
    const riskText = document.getElementById('riskText');
    const messageText = document.getElementById('messageText');
    const graphImage = document.getElementById('collisionGraph');
    const simText = document.getElementById('simText');

    decisionText.textContent = result.decision;
    decisionText.className = '';
    
    if(result.decision === 'SAFE') decisionText.classList.add('status-safe');
    else if(result.decision === 'UNSAFE') decisionText.classList.add('status-unsafe');
    else decisionText.classList.add('status-warning');

    riskText.textContent = result.risk;
    messageText.textContent = result.message;

    // Cache buster for the image
    graphImage.src = result.graph_url + '?t=' + new Date().getTime();

    // Show simulated positions
    const sim = result.simulation;
    simText.innerHTML = `
        Vehicle A: Speed ${sim.vehicle_a.speed} km/h, Position: ${sim.vehicle_a.position.toFixed(2)}m <br>
        Vehicle B: Speed ${sim.vehicle_b.speed} km/h, Position: ${sim.vehicle_b.position.toFixed(2)}m <br>
        Vehicle C: Speed ${sim.vehicle_c.speed} km/h, Position: ${sim.vehicle_c.position.toFixed(2)}m
    `;
}

function startAnimation(decision, speedA, speedB, speedC, distAB, distBC, oppPresent) {
    clearInterval(animationInterval);

    const carA = document.getElementById('carA');
    const carB = document.getElementById('carB');
    const carC = document.getElementById('carC');
    const overlay = document.getElementById('warningOverlay');

    overlay.classList.remove('flash-active');
    
    // Reset positions mathematically in UI
    let posA = 10; // base percentage
    let posB = 40; 
    let posC = 90;

    carA.style.left = posA + "%";
    carA.style.top = "95px"; // Default lane
    carB.style.left = posB + "%";
    carC.style.left = posC + "%";
    
    if (oppPresent === "Yes") {
        carC.style.display = "flex";
    } else {
        carC.style.display = "none";
    }

    // Animation Loop (update every 100ms)
    // Relative speeds to simulate movement
    let relSpeedA = (speedA / 100); 
    let relSpeedB = (speedB / 100);
    let relSpeedC = (speedC / 100);

    let allow_overtake = decision === 'SAFE';
    let overtaking = false;
    let collisionDetected = false;
    let dynamicUiUpdated = false;

    function moveVehicleAToOvertakeLane() {
        carA.style.top = "25px";
    }

    function increaseSpeedA() {
        relSpeedA = (speedA * 1.5) / 100;
    }

    animationInterval = setInterval(() => {
        if (collisionDetected) return;

        posA += relSpeedA;
        posB += relSpeedB;
        if (oppPresent === "Yes") {
            posC -= relSpeedC;
        }

        // Bound to container logic (simplistic map percentage wrap-around or stop)
        if(posB > 100) posB = 100;

        // Dynamic decision: if the opposite vehicle completely passes us, it becomes safe to overtake
        // Using posC < posA - 15 ensures visually they have cleared each other completely.
        if (oppPresent === "Yes" && posC < posA - 15) {
            allow_overtake = true;
            
            if (!dynamicUiUpdated) {
                const decisionText = document.getElementById('decisionText');
                decisionText.textContent = 'SAFE';
                decisionText.className = 'status-safe';
                document.getElementById('riskText').textContent = 'LOW';
                document.getElementById('messageText').textContent = 'Opposite vehicle passed. Safe to overtake';
                dynamicUiUpdated = true;
            }
        }

        // Overtake logic
        if (allow_overtake == true) {
            if (posA > posB - 15 && posA < posB + 15 && !overtaking) {
                moveVehicleAToOvertakeLane();
                increaseSpeedA();
                overtaking = true;
            } else if (posA > posB + 15 && overtaking) {
                // Return to lane
                carA.style.top = "95px";
                overtaking = false;
            }
        } else if (decision === 'UNSAFE' || decision === 'WARNING') {
            // Stop logic if getting too close
            if (posA > posB - 10) {
                posA = posB - 10; // Keep distance
            }
        }

        // Basic Collision visual detection (if math somehow fails or if we want to show it)
        if (posA >= posB - 5 && !allow_overtake && speedA > speedB) {
            overlay.classList.add('flash-active');
            collisionDetected = true;
        }
        
        // Ensure collision only happens if Vehicle A and Vehicle C actually overlap
        if (oppPresent === "Yes" && posA >= posC - 5 && posA <= posC + 15 && carA.style.top === "25px") {
            // Collision during overtake with opposite
            overlay.classList.add('flash-active');
            collisionDetected = true;
        }

        carA.style.left = posA + "%";
        carB.style.left = posB + "%";
        if (oppPresent === "Yes") carC.style.left = posC + "%";

        // Stop animation if A passes entirely
        if (posA > 110) clearInterval(animationInterval);

    }, 100);
}
