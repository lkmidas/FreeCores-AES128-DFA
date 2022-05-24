#include <iostream>
#include <cstdio>
#include <cstdint>

#include <verilated.h>

#include "Vaes_128.h"
#include "Vaes_128_aes_128.h"
#include "Vaes_128_one_round.h"
#include "Vaes_128_table_lookup.h"
#include "Vaes_128_S.h"
#include "Vaes_128_T.h"

// Utility functions
void print_int32_arr(uint32_t* arr, uint32_t len) {
    for (int i = len - 1; i >= 0; i--) {
        printf("%08x ", arr[i]);
    }
    printf("\n");
}

void copy_int32_arr(uint32_t* dest, uint32_t* src, uint32_t len) {
    for (int i = 0; i < len; i++) {
        dest[i] = src[i];
    }
}

void write_int32_arr_to_file(FILE* fp, uint32_t* arr, uint32_t len) {
    for (int i = len - 1; i >= 0; i--) {
        fprintf(fp, "%08x", arr[i]);
    }
    fprintf(fp, "\n");
}

uint8_t get_rand_byte() {
    uint32_t rand_val = 0;
    do {
        rand_val = rand();
    } while ((rand_val & 0xff) == 0);
    return (uint8_t)rand_val;
}
// Simulation functions for normal cases
void tick(Vaes_128* tb) {
    tb->clk = 0;
    tb->eval();
    tb->clk = 1;
    tb->eval();
    tb->clk = 0;
}

void aes128_encrypt(Vaes_128* tb) {
    uint32_t cycle = 0;

    // AES-128 completes in 21 clock cycles
    while (cycle < 21) {
        tick(tb);
        cycle++;
    }
}

// Simulation functions for faulty cases
void tick_fault_r8(Vaes_128* tb, uint32_t fault_value, uint32_t fault_table) {
    tb->clk = 0;
    tb->eval();

    // Fault injection at <fault_table> sbox's output
    Vaes_128_table_lookup* sboxes[4] = {
        tb->aes_128->r8->t0,
        tb->aes_128->r8->t1,
        tb->aes_128->r8->t2,
        tb->aes_128->r8->t3
    };

    sboxes[fault_table]->p0 ^= fault_value;

    tb->clk = 1;
    tb->eval();
    tb->clk = 0;
}

void aes128_encrypt_fault_r8(Vaes_128* tb, uint32_t fault_value, uint32_t fault_table) {
    uint32_t cycle = 0;

    // Each AES round is 2 cycles, as commented in round.v
    // First 2 cycles for key expansion
    // Which means: round r1 -> cycle 2, 3; round r2 -> cycle 4, 5; ... ; round rf -> cycle 20, 21
    // Round 8 -> cycle 16, 17, fault at least 1 cycle before
    while (cycle < 21) {
        if (cycle < 15) {
            tick(tb);
        } else {
            tick_fault_r8(tb, fault_value, fault_table);
        }
        cycle++;
    }
}

int main(int argc, char** argv) {

    bool verbose = false;

    // Only print secret key if verbose is false
    // Print all ciphertexts otherwise
    if (argc > 1) {
        if (std::string(argv[1]) == "-v") {
            verbose = true;
        }
    }
    
    Verilated::commandArgs(argc, argv);
    srand(time(NULL));

    Vaes_128* tb = new Vaes_128;

    // Generate key and plaintext randomly each run for better demonstration
    // For AES-128, keys are 128-bit (4-word) long
    for (int i = 3; i >= 0; i--) {
        tb->key[i] = (uint32_t)rand();
    }
    printf("[+] Secret Key (randomized, to be recovered):\n");
    print_int32_arr(tb->key, 4);

    // Plaintexts are 128-bit (4-word) long
    for (int i = 3; i >= 0; i--) {
        tb->state[i] = (uint32_t)rand();
    }
    if (verbose) {
        printf("[*] Plaintext:\n");
        print_int32_arr(tb->state, 4);
    }

    // First, we get the correct ciphertext as the golden example
    uint32_t golden_ciphertext[4];
    aes128_encrypt(tb);
    copy_int32_arr(golden_ciphertext, tb->out, 4);
    if (verbose) {
        printf("[*] Correct ciphertext:\n");
        print_int32_arr(tb->out, 4);
    }

    // To recover the last round key (K10), we need to have faulty outputs on final round rf
    // In order to corrupt round rf, a fault must be injected on round r8

    // Open a file to store faulty output to feed to phoenixAES
    FILE *fault_file = fopen("fault.txt", "w");

    // Write the golden ciphertext to output file first, as required by phoenixAES
    write_int32_arr_to_file(fault_file, golden_ciphertext, 4);

    // Faulting each sbox from 0 to 3 two times each on round 8
    uint32_t rand_val = 0;
    for (int i = 0; i < 4; i++) {
        aes128_encrypt_fault_r8(tb, get_rand_byte(), i);
        write_int32_arr_to_file(fault_file, tb->out, 4);
        if (verbose) {
            printf("[*] Faulted ciphertexts on sbox %d of round 9:\n", i);
            print_int32_arr(tb->out, 4);
        }

        aes128_encrypt_fault_r8(tb, get_rand_byte(), i);
        write_int32_arr_to_file(fault_file, tb->out, 4);
        if (verbose) {
            print_int32_arr(tb->out, 4);
        }
    }

    fclose(fault_file);

    return 0;
}
