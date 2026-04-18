def simulate_v2v(speed_a, speed_b, speed_c, dist_ab, dist_bc, time_sec=5):
    """
    Simulates the movement of 3 vehicles over a given time period.
    Initial positions:
    Vehicle A = 0
    Vehicle B = distance_ab
    Vehicle C = distance_ab + distance_bc (if opposite vehicle present, else just track position)
    Converts speed from km/h to m/s for accurate calculation.
    """
    
    # Convert km/h to m/s
    speed_a_ms = speed_a * (1000 / 3600)
    speed_b_ms = speed_b * (1000 / 3600)
    speed_c_ms = speed_c * (1000 / 3600)
    
    pos_a = 0
    pos_b = dist_ab
    pos_c = dist_ab + dist_bc
    
    # Simulate movement after time_sec
    new_pos_a = pos_a + (speed_a_ms * time_sec)
    new_pos_b = pos_b + (speed_b_ms * time_sec)
    # Opposing vehicle moves towards A and B
    new_pos_c = pos_c - (speed_c_ms * time_sec)
    
    return {
        "vehicle_a": {"speed": speed_a, "position": new_pos_a},
        "vehicle_b": {"speed": speed_b, "position": new_pos_b},
        "vehicle_c": {"speed": speed_c, "position": new_pos_c}
    }
