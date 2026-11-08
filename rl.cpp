#include <cstdio>
#include <iostream>
#include <format>

constexpr size_t char_max        =  255;
constexpr size_t min_buf_size    =  char_max + 1;
constexpr size_t  input_buf_size =  min_buf_size * 2;
constexpr size_t output_buf_size =  min_buf_size * 4;
char  input_buf[ input_buf_size];
char output_buf[output_buf_size];

constexpr size_t at_least_repeat = 3; // should be at least 2

int main(int argc, char *argv[])
{
/*
    size_t len;
    freopen(NULL, "rb", stdin);
    while (len =
         fread(buf, 1, 255, stdin)) {
        fwrite(buf, 1, len, stdout);
        std::cout << len;
    }
*/
    size_t read_bytes = 0;
    size_t written_bytes = 0;

    bool input_still_feeding = true;
    size_t input_end = 0;

    size_t input_buf_idx = 0, output_buf_idx = 0;
    size_t output_runlen_idx = 0;

    std::FILE* in_f = stdin;
    std::FILE* out_f = stdout;

    if (argc > 1) {
        in_f = std::fopen(argv[1], "r");
        if (in_f == NULL) { std::cerr << "File not found: " << argv[1] << "\n"; exit(1); }
        if (argc > 2) {
            out_f = std::fopen(argv[2], "w");
            if (out_f == NULL) { std::cerr << "Can not create file: " << argv[2] << "\n"; exit(1); }
        }
    }


    size_t print_skip = 0;
    auto print_statistics = [&] {
        constexpr size_t print_freq = 1000;
        if (!print_skip) {

            // print statistics
            constexpr const char* CLEAR_TO_EOL = "\x1b[K";
            std::fputs("\r", stderr);
            std::string stats = std::format(" Compression: {:5.1f}% {}/{}",
                    static_cast<float>(written_bytes)/static_cast<float>(read_bytes) * 100.0,
                    written_bytes, read_bytes);
            std::fputs(stats.c_str(), stderr);
            std::fputs(CLEAR_TO_EOL, stderr);
            std::fflush(stderr);
        }
        print_skip++; if (print_skip == print_freq) { print_skip = 0; }
    };

    auto read_in_file = [&] {
        input_end = std::fread(input_buf, 1, input_buf_size, in_f);
        read_bytes += input_end;
        input_still_feeding = input_end == input_buf_size;
        input_buf_idx = 0;
    };
    auto write_out_file = [&] {
        std::fwrite(output_buf, 1, output_buf_idx, out_f);
        //std::cerr << "in write:" << output_buf_idx << "\n";
        written_bytes += output_buf_idx;
        output_buf_idx = 0;
        print_statistics();
    };


    read_in_file();
    char last_elem = '\0';
    if (input_end > 0) {
        last_elem = input_buf[0] - 1;
    }

    size_t run_limit = 0;
    size_t run_idx = 0;

    do {
        //std::cerr << "Starting big do-while:\n";
        //std::cerr << "input_still_feeding: " << input_still_feeding << "\n";
        //std::cerr << "input_buf_idx: " << input_buf_idx << "\n";
        //std::cerr << "input_end: " << input_end << "\n";

    /* arbitrary data run */
    while( input_still_feeding || input_buf_idx < input_end ) {

        if ( input_buf_idx >  input_buf_size - min_buf_size)     { read_in_file();   }
        if (output_buf_idx > output_buf_size - min_buf_size - 1) { write_out_file(); }

        output_runlen_idx = output_buf_idx++;
        run_idx = 0;
        run_limit = input_still_feeding ? char_max : input_end - input_buf_idx;

        size_t same_length = 1;
        //std::cerr << "Arbitrary runlimit: " << run_limit << "\n";
        // TODO: we must ALWAYS enter this loop

        while (    run_idx < run_limit
                && same_length != at_least_repeat ) {
            run_idx++;
            same_length = input_buf[input_buf_idx]  == last_elem ? same_length + 1 : 1;
            last_elem = output_buf[output_buf_idx++] = input_buf[input_buf_idx++];
        }
        // TODO: this won't work with run_idx == run_limit case, something else needed
        output_buf[output_runlen_idx] = run_idx - at_least_repeat; // this run length
        // TODO: rare case, running_idx is 0 - or is it already done?
        if (run_idx < run_limit) {
            // Note: we have already read at least 2 identical elements, say 4 'A' letters:
            // AAAA - we go back to the first A, and use as the runlength,
            // and keep the second one: <runlength>AAA - the last two AAs can be overwritten
            output_buf_idx -= at_least_repeat; // next run length idx
            break; // go to repeating data block
        }
    }

    /* repeating data run */
    bool repeating_data_stored = false;
    //std::cerr << "input still feeding: " << input_still_feeding
    //          << " input_buf_idx: " << input_buf_idx << " input_end: " << input_end << "\n";
    while( input_still_feeding || input_buf_idx < input_end ) {

        //std::cerr << "Start repeating block:\n";
        if ( input_buf_idx >  input_buf_size - min_buf_size) {
            read_in_file();
        }
        if (output_buf_idx > output_buf_size - 2) {
            write_out_file();
        }

        auto& repeating_elem = last_elem;
        run_idx = 0;
        run_limit = input_still_feeding ? char_max : input_end - input_buf_idx;
        //std::cerr << "Repeating runlimit: " << run_limit << "\n";
        while (    run_idx < run_limit
                && input_buf[input_buf_idx] == repeating_elem ) {
            run_idx++;
            input_buf_idx++;
        }
        //std::cerr << "input_buf_idx: " << input_buf_idx << "\n";
        //std::cerr << "runlen: " << run_idx << " repeating_elem: " << repeating_elem << "\n";
        output_buf[output_buf_idx++] = run_idx ; // this run length
        //std::cerr << "after repeat_count: " << output_buf_idx << "\n";
        if (repeating_data_stored == false) {
            //std::cerr << "stored repeating char: " << output_buf[output_buf_idx++] << "\n";
            output_buf_idx++; // We skip over the second 'A' from the example above,
                              // it is already our repeating letter!
            repeating_data_stored  = true;
            //std::cerr << "in skip: " << output_buf_idx << "\n";
        }
        if (run_idx < char_max) {
            last_elem = input_buf[input_buf_idx];
            break; // go to arbitrary data block
        }
        //std::cerr << input_buf_idx << " , " << output_buf_idx << "\n";
    } // repeating data while end
    } while( input_still_feeding || input_buf_idx < input_end);
    print_skip = static_cast<size_t>(false);
    write_out_file();
    std::fputs("\n", stderr);
}
