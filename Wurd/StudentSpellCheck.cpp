#include "StudentSpellCheck.h"
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

SpellCheck* createSpellCheck() {
	return new StudentSpellCheck;
}

StudentSpellCheck::StudentSpellCheck() {
    create(m_root);
}

StudentSpellCheck::~StudentSpellCheck() {
    TrieNode* cur = m_root;

    if (!cur) {
        return;
    }

    for (int i = 0; i < 27; i++) {
        deleteChild(cur->child[i]);
    }

    delete m_root;
}

bool StudentSpellCheck::load(std::string dictionaryFile) {
    std::ifstream infile(dictionaryFile);

    if (!infile) {
        return false;
    }

    std::string word;

    while (getline(infile, word)) {
        std::string newWord;

        for (int i = 0; i < word.size(); i++) {
            // if letter or apostrophe
            if (isalpha(word[i]) || word[i] == '\'') {
                newWord += word[i];
            }
        }

        // add word to tree
        if (!newWord.empty()) {
            insert(newWord);
        }
    }

    return true;
}

bool StudentSpellCheck::spellCheck(std::string word, int max_suggestions, std::vector<std::string>& suggestions) {
    if (!search(word)) {
        suggestions.clear();
        
        for (int i = 0; i < word.size(); i++) {
            // convert to lowercase
            if (isupper(word[i])) {
                word[i] += 32;
            }
        }

        // add spelling suggestions
        for (int i = 0; i < word.size(); i++) {
            std::string newWord = word;

            for (int j = 0; j < 26; j++) {
                newWord[i] = 'a' + j;

                if (search(newWord) && max_suggestions > 0) {
                    suggestions.push_back(newWord);
                    max_suggestions--;
                }
            }

            newWord[i] = '\'';

            // if spelling suggestion is in tree
            if (search(newWord) && max_suggestions > 0) {
                suggestions.push_back(newWord);
                max_suggestions--;
            }
        }

        return false;
    }

    return true;
}

void StudentSpellCheck::spellCheckLine(const std::string& line, std::vector<SpellCheck::Position>& problems) {
    std::string word;
    Position wordPos;
    bool newWord = true;

    for (int i = 0; i < line.size(); i++) {
        if (i < line.size() - 1 && (isalpha(line[i]) || line[i] == '\'')) {
            if (newWord) {
                wordPos.start = i;
            }

            newWord = false;
            word += line[i];
        }
        
        // if last character of line
        else if (isalpha(line[i]) || line[i] == '\'') {
            if (newWord) {
                wordPos.start = i;
            }

            wordPos.end = i;
            word += line[i];
            
            if (!search(word)) {
                problems.push_back(wordPos);
            }
        }

        // if reached end of word
        else if (!word.empty()) {
            if (i == line.size() - 1) {
                wordPos.end = i;
            }
            
            else {
                wordPos.end = i - 1;
            }

            if (!search(word)) {
                problems.push_back(wordPos);
            }

            word.clear();
            newWord = true;
        }
    }
}

void StudentSpellCheck::insert(std::string word) {
    TrieNode* cur = m_root;
    int pos;

    for (int i = 0; i < word.size(); i++) {
        // convert to lowercase
        if (isupper(word[i])) {
            word[i] += 32;
        }

        if (word[i] == '\'') {
            pos = 26;
        }

        else {
            pos = word[i] - 'a';
        }

        // if word is not inserted
        if (!cur->child[pos]) {
            create(cur->child[pos]);
        }

        cur = cur->child[pos];
    }

    cur->endOfWord = true;
}

void StudentSpellCheck::create(TrieNode* &node) {
    node = new TrieNode;
    node->endOfWord = false;

    for (int i = 0; i < 27; i++) {
        node->child[i] = nullptr;
    }
}

bool StudentSpellCheck::search(std::string word) {
    TrieNode* cur = m_root;
    int pos;

    for (int i = 0; i < word.size(); i++) {
        // convert to lowercase
        if (isupper(word[i])) {
            word[i] += 32;
        }

        if (word[i] == '\'') {
            pos = 26;
        }

        else {
            pos = word[i] - 'a';
        }

        if (!cur->child[pos]) {
            return false;
        }

        cur = cur->child[pos];
    }

    return cur && cur->endOfWord;
}

void StudentSpellCheck::deleteChild(TrieNode* node) {
    if (!node) {
        return;
    }

    for (int i = 0; i < 27; i++) {
        if (node->child[i]) {
            deleteChild(node->child[i]);
        }
    }

    delete node;
}
