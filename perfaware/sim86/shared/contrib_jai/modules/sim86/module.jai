#import "Basic";

#load "sim86.jai";

#if DEBUG == true {
    SIM86 :: #library "sim86_shared_debug";
} else #if DEBUG == false {
    SIM86 :: #library "sim86_shared_release";
}

