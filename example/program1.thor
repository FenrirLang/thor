package main;

import "std.io";

func main(int argc, string[] argv) -> int {
    string business = std.input("Enter a name of a business : ");
    string location = std.input("Enter a location : ");
    string animal = std.input("Enter a type of animal : ");
    string verb = std.input("Enter a present continuous verb : ");

    std.println(" ");
    std.println("Mr. M Palin");
    std.println("Owner, %s" % [business]);
    std.println("%s, England" % [location]);
    std.println(" ");
    std.println("Dear Mr. Palin,");
    std.println("I am writing  this letter to complain about the service at");
    std.println("your %s." % [business]);
    std.println("I recently purchased a %s at this boutique. Once" % [animal]);
    std.println("I returned home, I discovered that the %s was " % [animal]);
    std.println("quite dead.  When I tried to return it, the clerk said that it was ");
    std.println("not dead, merely %s. I assure you that this" % [verb]);
    std.println("%s is not %s but is indeed" % [animal, verb]);
    std.println("quite dead.");
    std.println(" ");
    std.println("I appreciate your quick resolution of this matter.");
    std.println(" ");
    std.println("Sincerely,");
    std.println(" ");
    std.println("J. Cleese.");

    return 0;
}