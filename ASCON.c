#include <stdio.h>
#include <stdint.h>
#include <string.h>

typedef uint64_t bit64;

// Constants used in the add_constant function
bit64 constants[16] = { 0xf0, 0xe1, 0xd2, 0xc3, 0xb4, 0xa5, 0x96, 0x87,
                        0x78, 0x69, 0x5a, 0x4b, 0x3c, 0x2d, 0x1e, 0x0f };

// State array for the ASCON permutation
bit64 state[5] = { 0 }, t[5] = { 0 };

// Efficient bitwise rotation function
static inline bit64 rotate(bit64 x, int l) {
    return (x >> l) | (x << (64 - l));
}

// Adds round constants to the state
void add_constant(bit64 state[5], int i, int a) {
    if (12 - a + i >= 0 && 12 - a + i < 16) {
        state[2] ^= constants[12 - a + i];
    }
}

// S-box function for non-linear substitution
void sbox(bit64 x[5]) {
    bit64 t[5]; // Optimization: Use local variables instead of a global array

    // Initial XOR operations
    x[0] ^= x[4]; x[4] ^= x[3]; x[2] ^= x[1];

    // Compute intermediate values
    t[0] = ~x[0] & x[1];
    t[1] = ~x[1] & x[2];
    t[2] = ~x[2] & x[3];
    t[3] = ~x[3] & x[4];
    t[4] = ~x[4] & x[0];

    // Apply bitwise transformations
    x[0] ^= t[1]; x[1] ^= t[2]; x[2] ^= t[3]; x[3] ^= t[4]; x[4] ^= t[0];

    // Final transformations
    x[1] ^= x[0]; x[0] ^= x[4]; x[3] ^= x[2]; x[2] = ~x[2];
}

// Linear diffusion layer
void linear(bit64 state[5]) {
    state[0] ^= rotate(state[0], 19) ^ rotate(state[0], 28);
    state[1] ^= rotate(state[1], 61) ^ rotate(state[1], 39);
    state[2] ^= rotate(state[2], 1) ^ rotate(state[2], 6);
    state[3] ^= rotate(state[3], 10) ^ rotate(state[3], 17);
    state[4] ^= rotate(state[4], 7) ^ rotate(state[4], 41);
}

// Main ASCON permutation function, iterating 'a' rounds
void p(bit64 state[5], int a) {
    for (int i = 0; i < a; i++) {
        add_constant(state, i, a);
        sbox(state);
        linear(state);
    }
}

// ASCON-128 initialization phase
void initialization(bit64 state[5], bit64 key[2]) {
    p(state, 12); // Perform 12 rounds of permutation
    state[3] ^= key[0]; // XOR with the secret key
    state[4] ^= key[1];
}

// Associated Data Processing (for authenticated encryption)
void associated_data(bit64 state[5], int length, bit64 associated_data_text[]) {
    for (int i = 0; i < length; i++) {
        state[0] ^= associated_data_text[i]; // XOR each block of associated data
        p(state, 6); // Perform 6 permutation rounds
    }
    state[0] ^= 0x0000000000000001; // XOR with a fixed bit to mark completion
}

// Finalization step to generate authentication tag
void finalization(bit64 state[5], bit64 key[2]) {
    state[1] ^= key[0]; // XOR with the secret key
    state[2] ^= key[1];
    p(state, 12); // Perform 12 rounds of permutation
    state[3] ^= key[0];
    state[4] ^= key[1];
}

// ASCON-128 encryption function
void encrypt(bit64 state[5], int length, bit64 plaintext[], bit64 ciphertext[], bit64* tag1, bit64* tag2) {
    ciphertext[0] = plaintext[0] ^ state[0]; // Initial XOR operation

    for (int i = 1; i < length; i++) {
        p(state, 6); // Apply 6 rounds of permutation between blocks
        ciphertext[i] = plaintext[i] ^ state[0];
        state[0] = ciphertext[i]; // Update the state with ciphertext
    }

    finalization(state, (bit64[]) { 0, 0 }); // 🔥 Error: Must use 'key' instead of zero
    *tag1 = state[3];
    *tag2 = state[4];
}

// Verifies if the computed authentication tag matches the received tag
int verify_tag(bit64 computed_tag1, bit64 computed_tag2, bit64 received_tag1, bit64 received_tag2) {
    return (computed_tag1 == received_tag1) && (computed_tag2 == received_tag2);
}

// ASCON-128 decryption function
int decrypt(bit64 state[5], int length, bit64 plaintext[], bit64 ciphertext[], bit64 received_tag1, bit64 received_tag2) {
    plaintext[0] = ciphertext[0] ^ state[0]; // Initial XOR operation

    for (int i = 1; i < length; i++) {
        p(state, 6);
        plaintext[i] = ciphertext[i] ^ state[0];
        state[0] = ciphertext[i];
    }

    finalization(state, (bit64[]) { 0, 0 }); // 🔥 Error: Must use 'key' instead of zero

    if (!verify_tag(state[3], state[4], received_tag1, received_tag2)) {
        printf("ERROR: Authentication failed! Data is invalid.\n");
        return 0; // Authentication failure
    }

    return 1; // Authentication successful
}

int main() {
    bit64 nonce[2] = { 0x0000000000000001, 0x0000000000000002 };
    bit64 key[2] = { 0 };
    bit64 IV = 0x80400c0600000000;
    bit64 plaintext[] = { 0x1234567890abcdef, 0x1234567890abcdef };
    bit64 ciphertext[2] = { 0 };
    bit64 tag1, tag2;
    bit64 associated_data_text[] = { 0x787878, 0x878787, 0x09090 };

    // Initialize the state
    state[0] = IV;
    state[1] = key[0];
    state[2] = key[1];
    state[3] = nonce[0];
    state[4] = nonce[1];

    initialization(state, key);
    associated_data(state, 3, associated_data_text);
    encrypt(state, 2, plaintext, ciphertext, &tag1, &tag2);

    printf("\nciphertext: %016llx %016llx\n", ciphertext[0], ciphertext[1]);
    printf("tag: %016llx %016llx\n", tag1, tag2);

    // Reset the state before decryption
    state[0] = IV;
    state[1] = key[0];
    state[2] = key[1];
    state[3] = nonce[0];
    state[4] = nonce[1];
      
    initialization(state, key);
    associated_data(state, 3, associated_data_text);

    if (decrypt(state, 2, plaintext, ciphertext, tag1, tag2)) {
        printf("\nplaintext: %016llx %016llx\n", plaintext[0], plaintext[1]);
    }
    else {
        printf("Decryption failed due to invalid tag.\n");
    }

    return 0;
}
