#include "crypto.h"
#include <random>
#include <sstream>
#include <iomanip>

// Crypto++
#include "sha.h"
#include "hex.h"
#include "osrng.h"
#include "filters.h"

std::string Crypto::generateSalt() {
    CryptoPP::AutoSeededRandomPool rng;
    uint8_t salt[16];
    rng.GenerateBlock(salt, sizeof(salt));

    std::string result;
    CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(result));
    encoder.Put(salt, sizeof(salt));
    encoder.MessageEnd();

    return result;
}

std::string Crypto::hashPassword(const std::string& password,
                                  const std::string& salt)
{
    // SHA256(salt + password)
    std::string input = salt + password;
    std::string digest;

    CryptoPP::SHA256 hash;
    CryptoPP::StringSource ss(
        input, true,
        new CryptoPP::HashFilter(
            hash,
            new CryptoPP::HexEncoder(
                new CryptoPP::StringSink(digest)
            )
        )
    );

    return digest;
}

bool Crypto::verifyPassword(const std::string& password,
                             const std::string& salt,
                             const std::string& stored_hash)
{
    std::string computed = hashPassword(password, salt);
    return computed == stored_hash;
}

std::string Crypto::generateToken() {
    CryptoPP::AutoSeededRandomPool rng;
    uint8_t token[32];
    rng.GenerateBlock(token, sizeof(token));

    std::string result;
    CryptoPP::HexEncoder encoder(new CryptoPP::StringSink(result));
    encoder.Put(token, sizeof(token));
    encoder.MessageEnd();

    return result;
}