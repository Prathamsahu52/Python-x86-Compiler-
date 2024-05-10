class RabinKarp:
    def __init__(self, pattern):
        self.pattern = pattern
        self.prime = 101  # Prime number for hashing
        self.base = 256   # Base for hashing
        self.pattern_hash = self.create_hash(pattern, len(pattern))

    def create_hash(self, string, length):
        hash_value = 0
        for i in range(length):
            hash_value += ord(string[i]) * (self.base ** (length - i - 1))
        return hash_value

    def rehash(self, old_hash, old_char, new_char, pattern_length):
        new_hash = old_hash - ord(old_char) * (self.base ** (pattern_length - 1))
        new_hash = new_hash * self.base + ord(new_char)
        return new_hash

    def search(self, text):
        pattern_length = len(self.pattern)
        text_length = len(text)

        text_hash = self.create_hash(text, pattern_length)

        for i in range(text_length - pattern_length + 1):
            if text_hash == self.pattern_hash:
                if text[i:i + pattern_length] == self.pattern:
                    print("Rabin-Karp: Pattern found at index", i)

            if i < text_length - pattern_length:
                text_hash = self.rehash(text_hash, text[i], text[i + pattern_length], pattern_length)


class KMP:
    def __init__(self, pattern):
        self.pattern = pattern
        self.prefix_table = self.compute_prefix_table(pattern)

    def compute_prefix_table(self, pattern):
        prefix_table = [0] * len(pattern)
        length = 0
        i = 1
        while i < len(pattern):
            if pattern[i] == pattern[length]:
                length += 1
                prefix_table[i] = length
                i += 1
            else:
                if length != 0:
                    length = prefix_table[length - 1]
                else:
                    prefix_table[i] = 0
                    i += 1
        return prefix_table

    def search(self, text):
        text_length = len(text)
        pattern_length = len(self.pattern)
        i = 0
        j = 0
        while i < text_length:
            if self.pattern[j] == text[i]:
                i += 1
                j += 1
            if j == pattern_length:
                print("KMP: Pattern found at index", i - j)
                j = self.prefix_table[j - 1]
            elif i < text_length and self.pattern[j] != text[i]:
                if j != 0:
                    j = self.prefix_table[j - 1]
                else:
                    i += 1


# Test the algorithms
pattern = "abc"
text = "abcbacabcababcabc"
rk = RabinKarp(pattern)
rk.search(text)

kmp = KMP(pattern)
kmp.search(text)
