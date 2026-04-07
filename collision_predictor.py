import matplotlib
matplotlib.use('Agg') # Use non-interactive backend
import matplotlib.pyplot as plt
import numpy as np
import os

def generate_collision_graph(speed_a, speed_b, speed_c, dist_ab, dist_bc, opp_present):
    """
    Plots the distance between A-B and A-C over time.
    X axis = time
    Y axis = distance
    """
    # Convert km/h to m/s
    sa_ms = speed_a * (5/18)
    sb_ms = speed_b * (5/18)
    sc_ms = speed_c * (5/18)

    time = np.linspace(0, 10, 100) # 0 to 10 seconds

    # Calculate distance between A and B over time
    # Dist_AB(t) = (pos_b + sb_ms*t) - (pos_a + sa_ms*t) = dist_ab + (sb_ms - sa_ms)*t
    dist_a_b_t = dist_ab + (sb_ms - sa_ms) * time
    
    # Calculate distance between A and C over time
    # Dist_AC(t) = (pos_c - sc_ms*t) - (pos_a + sa_ms*t) = (dist_ab + dist_bc) - (sc_ms + sa_ms)*t
    initial_dist_ac = dist_ab + dist_bc
    dist_a_c_t = initial_dist_ac - (sc_ms + sa_ms) * time if opp_present == "Yes" else None

    plt.figure(figsize=(6, 4))
    plt.plot(time, dist_a_b_t, label='Distance A-B', color='blue')
    
    if opp_present == "Yes":
        plt.plot(time, dist_a_c_t, label='Distance A-C (Opposite)', color='red')
    
    plt.axhline(0, color='black', linewidth=1, linestyle='--')
    plt.fill_between(time, 0, dist_a_b_t, where=(dist_a_b_t <= 0), color='red', alpha=0.3, label='Collision A-B')
    
    if opp_present == "Yes":
        plt.fill_between(time, 0, dist_a_c_t, where=(dist_a_c_t <= 0), color='orange', alpha=0.3, label='Collision A-C')

    plt.title('Collision Prediction')
    plt.xlabel('Time (seconds)')
    plt.ylabel('Distance (meters)')
    plt.legend()
    plt.grid(True)

    # Save graph
    save_path = os.path.join(os.path.dirname(__file__), 'static', 'collision.png')
    plt.savefig(save_path)
    plt.close()
