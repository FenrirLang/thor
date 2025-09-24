package main;

import "std.io";
import "math";

// The main function
func main(string[] args) -> int {
    std.println("Hello, World!");

    boolean is_running = true;
    while (is_running) {
        std.println("Testing Options:");
        std.println("- math");
        string in = std.input("What would you like to test? ('quit' to quit) ");
        if (in == "quit") {
            is_running = false;
        } 
        else if (in == "math") {
            test_math();
        }
    }

    std.println("Finished Testing...");

    return 0;
}

func test_math() -> void {
    std.println("[TESTING] basic.thor");
    int a = 5;
    int b = 2;

    int sum = math.add(a, b);
    int diff = math.sub(a, b);

    std.println("%s + %s = %s" % [a, b, sum]);
    std.println("%s - %s = %s" % [a, b, diff]);
}