def check_rule_1(opp_present):
    return opp_present == "Yes"

def check_rule_2(dist_bc):
    return dist_bc < 20

def check_rule_3(speed_a, speed_b):
    return speed_a <= speed_b

def check_rule_4(road_type):
    return road_type == "Curve"

def check_rule_5(visibility):
    return visibility == "Poor"

def check_rule_6(dist_ab, speed_a, speed_b, opp_present):
    # If no opposite vehicle, speed is sufficient, and distance isn't critically low
    return dist_ab >= 10 and speed_a > speed_b and opp_present == "No"

def check_rule_7(dist_ab, opp_present):
    return 10 <= dist_ab <= 20 and opp_present == "Yes"
