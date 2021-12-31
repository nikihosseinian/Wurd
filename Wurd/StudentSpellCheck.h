#ifndef STUDENTSPELLCHECK_H_
#define STUDENTSPELLCHECK_H_

#include "SpellCheck.h"
#include <string>
#include <vector>

class StudentSpellCheck : public SpellCheck {
public:
    StudentSpellCheck();
	virtual ~StudentSpellCheck();
	bool load(std::string dict_file);
	bool spellCheck(std::string word, int maxSuggestions, std::vector<std::string>& suggestions);
	void spellCheckLine(const std::string& line, std::vector<Position>& problems);

private:
    struct TrieNode {
        TrieNode* child[27];
        bool endOfWord;
    };
    
    void insert(std::string word);
    void create(TrieNode* &node);
    bool search(std::string word);
    void deleteChild(TrieNode* node);
    
    TrieNode* m_root;
};

#endif  // STUDENTSPELLCHECK_H_
