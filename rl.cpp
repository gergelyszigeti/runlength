#include <cstdio>
#include <iostream>

constexpr size_t char_max        =  255;
constexpr size_t min_buf_size    =  char_max + 1;
constexpr size_t  input_buf_size =  min_buf_size * 2;
constexpr size_t output_buf_size =  min_buf_size * 4;
char  input_buf[ input_buf_size];
char output_buf[output_buf_size];

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
    size_t output_runlen = 0;

    auto read_stdin = [&] {
        input_end = fread(input_buf, 1, input_buf_size, stdin);
        input_still_feeding = input_end == input_buf_size;
        input_buf_idx = 0;
    };
    auto write_stdout = [&] {
        fwrite(output_buf, 1, output_buf_idx, stdout);
        output_buf_idx = 0;
    };

    read_stdin();
    char last_elem = '\0';
    if (input_end) {
        last_elem = input_buf[0] - 1;
    }


    size_t run_limit = 0;
    size_t run_idx = 0;

    do {

    /* arbitrary data run */
    while( input_still_feeding || input_buf_idx < input_end ) {

        if ( input_buf_idx >  input_buf_size - min_buf_size)     { read_stdin();   }
        if (output_buf_idx > output_buf_size - min_buf_size - 1) { write_stdout(); }

        output_runlen = output_buf_idx++;
        run_idx = 0;
        run_limit = input_still_feeding ? char_max : input_end - input_buf_idx;
        //std::cerr << "Arbitrary runlimit: " << run_limit << "\n";
        // TODO: we must ALWAYS enter this loop
        while (    run_idx < run_limit
                && input_buf[input_buf_idx] != last_elem ) {
            last_elem = output_buf[output_buf_idx++] = input_buf[input_buf_idx++];
            run_idx++;
        }
        // TODO: here we should decrease run_idx by one (already repeating elem)
        // TODO: if run_idx - 1 is not 0, decrease output idx also
        //       (and count already repeating element in the next block)
        output_buf[output_runlen] = run_idx; // this run length
        // TODO: rare case, running_idx is 0 - or is it already done?
        if (run_idx < char_max) { break; } // go to repeating data block
    }

    /* repeating data run */
    bool repeating_data_stored = false;
    while( input_still_feeding || input_buf_idx < input_end ) {

        if ( input_buf_idx >  input_buf_size - min_buf_size) {
            // read new input
            //input_buf_idx = 0;
            read_stdin();

            //std::cerr << "HELLO\n";
        }
        if (output_buf_idx > output_buf_size - 2) {
            // write out current output
            //output_buf_idx = 0;
            write_stdout();
            //std::cerr << "BELLO\n";
        }

        run_idx = 0;
        run_limit = input_still_feeding ? char_max : input_end - input_buf_idx;
        //std::cerr << "Repeating runlimit: " << run_limit << "\n";
        // note, last_elem comes from the arbitrary data block
        while (    run_idx < run_limit
                && input_buf[input_buf_idx] == last_elem ) {
            input_buf_idx++;
            run_idx++;
        }
        //std::cerr << "input_buf_idx: " << input_buf_idx << "\n";
        output_buf[output_buf_idx++] = run_idx; // this run length
        if (repeating_data_stored == false) {
            output_buf[output_buf_idx++] = last_elem; // repeat this data when decoding
            repeating_data_stored  = true;
        }
        if (run_idx < char_max) { break; } // go to arbitrary data block
        //std::cerr << input_buf_idx << " , " << output_buf_idx << "\n";
    }
    } while( input_still_feeding || input_buf_idx < input_end);
    write_stdout();
}
