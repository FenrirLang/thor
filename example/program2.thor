package main;

import "std.io";

const int PALMS_PER_CUBIT = 6;
const int FINGERS_PER_PALM = 4;
const int FINGERS_PER_CUBIT = PALMS_PER_CUBIT * FINGERS_PER_PALM;

const int RICE_PER_FINGER = 1;

func to_fingers(int cubits, int palms, int fingers) -> int {
    return cubits * FINGERS_PER_CUBIT + palms * FINGERS_PER_PALM + fingers;
}

func fromFingers(int totalFingers, int& cubits, int& palms, int& fingers) -> void {
    cubits = totalFingers / FINGERS_PER_CUBIT;
    totalFingers %= FINGERS_PER_CUBIT;
    palms = totalFingers / FINGERS_PER_PALM;
    fingers = totalFingers % FINGERS_PER_PALM;
}

func main(int argc, string[] argv) -> int {
    
    return 0;
}