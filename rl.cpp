#include <cstdio>
#include <iostream>

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
    bool input_still_feeding = true;
    size_t input_end = 0;

    size_t input_buf_idx = 0, output_buf_idx = 0;
    size_t output_runlen_idx = 0;

    auto read_stdin = [&] {
        input_end = fread(input_buf, 1, input_buf_size, stdin);
        input_still_feeding = input_end == input_buf_size;
        input_buf_idx = 0;
    };
    auto write_stdout = [&] {
        fwrite(output_buf, 1, output_buf_idx, stdout);
        //std::cerr << "in write:" << output_buf_idx << "\n";
        output_buf_idx = 0;
    };


    read_stdin();
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

        if ( input_buf_idx >  input_buf_size - min_buf_size)     { read_stdin();   }
        if (output_buf_idx > output_buf_size - min_buf_size - 1) { write_stdout(); }

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
        // TODO: here we should decrease run_idx by one (already repeating elem)
        // TODO: if run_idx - 1 is not 0, decrease output idx also
        //       (and count already repeating element in the next block)
        output_buf[output_runlen_idx] = run_idx - at_least_repeat; // this run length
        // TODO: rare case, running_idx is 0 - or is it already done?
        if (run_idx < char_max) {
            // Note: we have already read at least 2 identical elements, say 4 'A' letters:
            // AAAA - we go back to the first A, and use as the runlength,
            // and keep the second one: <runlength>AAA - the last two AAs can be overwritten
            output_buf_idx -= at_least_repeat; // next run length idx
            break; // go to repeating data block
        }
    }

    /* repeating data run */
    bool repeating_data_stored = false;
    while( input_still_feeding || input_buf_idx < input_end ) {

        //std::cerr << "Start repeating block:\n";
        if ( input_buf_idx >  input_buf_size - min_buf_size) {
            read_stdin();
        }
        if (output_buf_idx > output_buf_size - 2) {
            write_stdout();
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
        output_buf[output_buf_idx++] = run_idx ; // this run length
        //std::cerr << "after repeat_count: " << output_buf_idx << "\n";
        if (repeating_data_stored == false) {
            output_buf_idx++; // We skip over the second 'A' from the example above,
                              // it is already our repeating letter!
            repeating_data_stored  = true;
            std::cerr << "in skip: " << output_buf_idx << "\n";
        }
        if (run_idx < char_max) {
            last_elem = input_buf[input_buf_idx];
            break; // go to arbitrary data block
        }
        //std::cerr << input_buf_idx << " , " << output_buf_idx << "\n";
    } // repeating data while end
    } while( input_still_feeding || input_buf_idx < input_end);
    write_stdout();
}
