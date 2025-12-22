package main

import "core:strings"
import "core:fmt"
import "core:os/os2"

main :: proc() {
    fmt.printfln("[EXT] Extract data from json");
    path := "./yelp_dataset.json";

    //TODO: Need to grab all possible categories
    if data, err := os2.read_entire_file_from_path(path, context.temp_allocator); err == nil {
        jsons := transmute(string)data;
        lines, err := strings.split_lines(jsons, context.temp_allocator);

        /*
        for line in lines[0:5] {
            lo, ok_lo := strings.substring_to(line, strings.index(line, "\"address"));
            hi, ok_hi := strings.substring_from(line, strings.index(line, "\"attribute"));

            final, err_final := strings.concatenate({ lo, hi }, context.temp_allocator);
            fmt.printfln(" -> {} ", final);
        }
        */

        for line in lines[:] {
            start := strings.index(line, "categories");
            res, ok_res := strings.substring_from(line, start);
            end := strings.index(res, "\",");
            cat, ok_cat := strings.substring(line, start+13, start+end);
            fmt.printfln(" -> ({}) {} ", len(cat), cat);
        }
        return;
    }

    fmt.printfln("[EXT] Could not read file at {}", path);
}
