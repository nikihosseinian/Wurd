#include "StudentTextEditor.h"
#include "Undo.h"
#include <string>
#include <vector>
#include <list>
#include <iostream>
#include <fstream>

TextEditor* createTextEditor(Undo* un) {
	return new StudentTextEditor(un);
}

StudentTextEditor::StudentTextEditor(Undo* undo)
 : TextEditor(undo) {
     m_contents.push_back("");
     resetPos();
     m_undo = false;
}

StudentTextEditor::~StudentTextEditor() {

}

bool StudentTextEditor::load(std::string file) {
    std::ifstream infile(file);

    if (!infile) {
        return false;
    }
    
    reset();
    
    std::string line;
    char ch;
    
    while (infile.get(ch)) {
        // if valid character
        if (ch != '\r' && ch != '\n') {
            line += ch;
        }
        
        // if reached end of line
        if (ch == '\n') {
            m_contents.push_back(line);
            line.clear();
        }
    }
    
    resetPos();
    return true;
}

bool StudentTextEditor::save(std::string file) {
    std::ofstream outfile(file);

    if (!outfile) {
        return false;
    }

    std::list<std::string>::iterator it;
    
    for (it = m_contents.begin(); it != m_contents.end(); it++) {
        outfile << *it << '\n';
    }

    return true;
}

void StudentTextEditor::reset() {
    m_contents.clear();
    resetPos();
    getUndo()->clear();
    m_undo = false;
}

void StudentTextEditor::move(Dir dir) {
    if (dir == Dir::UP && m_row > 0) {
        decRow();
        
        // if empty line
        if (m_it->size() == 0) {
            m_col = 0;
        }
        
        if (m_col > m_it->size()) {
            m_col = m_it->size();
        }
    }
    
    if (dir == Dir::DOWN && m_row < m_contents.size() - 1) {
        incRow();
        
        // if empty line
        if (m_it->size() == 0) {
            m_col = 0;
        }
        
        if (m_col > m_it->size()) {
            m_col = m_it->size();
        }
    }
    
    if (dir == Dir::LEFT && (m_row > 0 || m_col > 0)) {
        if (m_col == 0) {
            decRow();
            m_col = m_it->size();
        }
        
        else {
            m_col--;
        }
    }
    
    if (dir == Dir::RIGHT && (m_row < m_contents.size() - 1 || m_col < m_it->size())) {
        if (m_col == m_it->size()) {
            incRow();
            m_col = 0;
        }

        else {
            m_col++;
        }
    }
    
    if (dir == Dir::HOME) {
        m_col = 0;
    }
    
    if (dir == Dir::END) {
        m_col = m_it->size();
    }
}

void StudentTextEditor::del() {
    // if there is nothing to delete
    if (m_row == m_contents.size() - 1 && m_col == m_it->size()) {
        return;
    }
    
    std::string line = *m_it;
    
    // if position is at end of line
    if (m_col == m_it->size()) {
        incRow();
        line += *m_it;
        m_it = m_contents.erase(m_it);
        
        decRow();
        *m_it = line;
        
        if (!m_undo) {
            getUndo()->submit(Undo::Action::JOIN, m_row, m_col);
        }
    }
    
    else {
        char ch = line[m_col];
        line.erase(m_col, 1);
        *m_it = line;
        
        if (!m_undo) {
            getUndo()->submit(Undo::Action::DELETE, m_row, m_col, ch);
        }
    }
}

void StudentTextEditor::backspace() {
    // if nothing to backspace
    if (m_row == 0 && m_col == 0) {
        return;
    }

    std::string line = *m_it;

    // if position is not at first character
    if (m_col > 0) {
        char ch = line[m_col - 1];
        line.erase(m_col - 1, 1);
        *m_it = line;
        m_col--;
        
        if (!m_undo) {
            getUndo()->submit(Undo::Action::DELETE, m_row, m_col, ch);
        }
    }

    // if position is at first character or line is empty
    else if (m_col == 0 || line.size() == 0) {
        decRow();
        line = *m_it;
        m_col = line.size();

        incRow();
        line += *m_it;
        m_it = m_contents.erase(m_it);
        
        decRow();
        *m_it = line;
        
        if (!m_undo) {
            getUndo()->submit(Undo::Action::JOIN, m_row, m_col);
        }
    }
}

void StudentTextEditor::insert(char ch) {
    std::string line = *m_it;

    // if inserting tab
    if (ch == '\t') {
        line.insert(m_col, "    ");
        m_col += 4;
    }

    else {
        line.insert(line.begin() + m_col, ch);
        m_col++;
    }
    
    *m_it = line;
    
    if (!m_undo) {
        getUndo()->submit(Undo::Action::INSERT, m_row, m_col, ch);
    }
}

void StudentTextEditor::enter() {
    if (!m_undo) {
        getUndo()->submit(Undo::Action::SPLIT, m_row, m_col);
    }
    
    std::string line = *m_it;
    std::string newLine = line.substr(m_col);
    *m_it = line.substr(0, m_col);
    
    incRow();
    m_col = 0;
    
    // add line break
    m_it = m_contents.insert(m_it, newLine);
}

void StudentTextEditor::getPos(int& row, int& col) const {
    row = m_row;
    col = m_col;
}

int StudentTextEditor::getLines(int startRow, int numRows, std::vector<std::string>& lines) const {
    if (startRow < 0 || numRows < 0 || startRow > m_contents.size()) {
        return -1;
    }
    
    lines.clear();
    int count = 0;
    
    if (startRow == m_contents.size()) {
        return count;
    }

    std::list<std::string>::const_iterator it = m_contents.begin();
    advance(it, startRow);

    // add available lines
    for (int i = 0; i < numRows; i++) {
        if (startRow + i < m_contents.size()) {
            lines.push_back(*it);
            it++;
            count++;
        }
    }
    
    return count;
}

void StudentTextEditor::undo() {
    int row;
    int col;
    int count;
    std::string text;
    Undo::Action action = getUndo()->get(row, col, count, text);

    if (action == Undo::Action::ERROR) {
        return;
    }
    
    m_row = row;
    m_col = col;
    m_it = m_contents.begin();
    advance(m_it, m_row);
    m_undo = true;
    
    // if undoing delete or backspace
    if (action == Undo::Action::INSERT) {
        for (int i = 0; i < text.size(); i++) {
            insert(text[i]);
        }
    }
    
    // if undoing insert
    if (action == Undo::Action::DELETE) {
        for (int i = 0; i < count; i++) {
            del();
        }
    }
    
    // if undoing join
    if (action == Undo::Action::SPLIT) {
        enter();
    }

    // if undoing line break
    if (action == Undo::Action::JOIN) {
        del();
    }

    m_row = row;
    m_col = col;
    m_it = m_contents.begin();
    advance(m_it, m_row);
    m_undo = false;
}

void StudentTextEditor::resetPos() {
    m_row = 0;
    m_col = 0;
    m_it = m_contents.begin();
}

void StudentTextEditor::incRow() {
    m_row++;
    m_it++;
}

void StudentTextEditor::decRow() {
    m_row--;
    m_it--;
}
