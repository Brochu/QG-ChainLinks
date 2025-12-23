package main

import "core:encoding/json"
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

        dict := make(map[string]int);
        //for line in lines[370:375] {
        for line in lines[:] {
            parser := json.make_parser_from_string(line, json.DEFAULT_SPECIFICATION, false, context.temp_allocator);
            val, err := json.parse_object(&parser)

            obj, ok_obj := val.(json.Object);
            if (!ok_obj) { continue }
            cat_line, ok_line := obj["categories"].(json.String);
            if (!ok_line) { continue }

            cats, err_cats := strings.split(cat_line, ", ", context.temp_allocator);
            for c in cats {
                dict[c] += 1;
            }
        }

        fmt.printfln("[EXT] dict len = {}", len(dict));
        for k, v in dict {
            if v > 2000 {
                fmt.printfln(" - [%v] -> %v", v, k);
            }
        }
        return;
    }

    fmt.printfln("[EXT] Could not read file at {}", path);
}
