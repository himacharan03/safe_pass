from rules import *

def decide_overtake(speed_a, speed_b, speed_c, dist_ab, dist_bc, opp_present, road_type, visibility, pos_a=0, pos_b=0, pos_c=0):
    """
    Evaluates overtaking safety based on predefined rules.
    Returns: decision, risk, message
    """
    if opp_present == "Yes" and pos_c < pos_a - 15:
        return "SAFE", "LOW", "Opposite vehicle passed. Safe to overtake"

    if check_rule_1(opp_present):
        return "UNSAFE", "HIGH", "Opposite vehicle is present!"
    
    if check_rule_2(dist_bc) and opp_present == "Yes":
        return "UNSAFE", "HIGH", "Vehicle C is too close!"
        
    if check_rule_4(road_type):
        return "UNSAFE", "HIGH", "Cannot overtake on a curve!"
        
    if check_rule_5(visibility):
        return "UNSAFE", "HIGH", "Visibility is completely poor!"

    if check_rule_3(speed_a, speed_b):
        return "UNSAFE", "LOW", "Cannot overtake. Speed A <= Speed B."
    
    if check_rule_6(dist_ab, speed_a, speed_b, opp_present):
        return "SAFE", "LOW", "Safe to overtake."
        
    if check_rule_7(dist_ab, opp_present):
        return "WARNING", "MEDIUM", "Distance is marginal. Proceed with caution."

    return "UNSAFE", "MEDIUM", "Conditions are not optimal for overtaking."
