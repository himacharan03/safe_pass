from flask import Flask, render_template, request, jsonify
from decision_engine import decide_overtake
from collision_predictor import generate_collision_graph
from v2v_simulator import simulate_v2v

app = Flask(__name__)

@app.route('/')
def home():
    return render_template('index.html')

@app.route('/predict', methods=['POST'])
def predict():
    data = request.json
    
    speed_a = float(data['speed_a'])
    speed_b = float(data['speed_b'])
    speed_c = float(data['speed_c'])
    dist_ab = float(data['dist_ab'])
    dist_bc = float(data['dist_bc'])
    opp_present = data['opp_present']
    road_type = data['road_type']
    visibility = data['visibility']

    pos_a = 0
    pos_b = dist_ab
    pos_c = dist_ab + dist_bc

    decision, risk, message = decide_overtake(
        speed_a, speed_b, speed_c, dist_ab, dist_bc, opp_present, road_type, visibility, pos_a, pos_b, pos_c
    )
    
    generate_collision_graph(speed_a, speed_b, speed_c, dist_ab, dist_bc, opp_present)
    
    simulation = simulate_v2v(speed_a, speed_b, speed_c, dist_ab, dist_bc)

    allow_overtake = True if (decision == "SAFE" or (opp_present == "Yes" and pos_c < pos_a - 15)) else False

    return jsonify({
        'decision': decision,
        'risk': risk,
        'message': message,
        'vehicle_a_position': pos_a,
        'vehicle_b_position': pos_b,
        'vehicle_c_position': pos_c,
        'allow_overtake': allow_overtake,
        'simulation': simulation,
        'graph_url': '/static/collision.png'
    })

if __name__ == '__main__':
    app.run(debug=True)
