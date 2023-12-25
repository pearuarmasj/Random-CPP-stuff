#include <iostream>
#include <string>

std::string encrypt(std::string text, int shift) {
    std::string result = "";
    for (int i = 0; i < text.length(); i++) {
        if (isupper(text[i])) {
            result += char(int(text[i] + shift - 65) % 26 + 65);
        }
        else {
            result += char(int(text[i] + shift - 97) % 26 + 97);
        }
    }
    return result;
}

std::string decrypt(std::string text, int shift) {
    std::string result = "";
    for (int i = 0; i < text.length(); i++) {
        if (isupper(text[i])) {
            result += char(int((text[i] - shift - 65 + 26) % 26) + 65);
        }
        else {
            result += char(int((text[i] - shift - 97 + 26) % 26) + 97);
        }
    }
    return result;
}

int main() {
    std::string text;
    int shift;
    std::cout << "Enter text: ";
    std::getline(std::cin, text);
    std::cout << "Enter shift: ";
    std::cin >> shift;
    std::string encrypted = encrypt(text, shift);
    std::cout << "Encrypted text: " << encrypted << std::endl;
    std::string decrypted = decrypt(encrypted, shift);
    std::cout << "Decrypted text: " << decrypted << std::endl;
    return 0;
}