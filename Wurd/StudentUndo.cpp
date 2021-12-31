#include "StudentUndo.h"
#include <string>
#include <stack>

Undo* createUndo()
{
	return new StudentUndo;
}

void StudentUndo::submit(const Action action, int row, int col, char ch) {
    // batch same actions
    if (!m_actions.empty() && m_actions.top().action == action && m_actions.top().row == row &&
        action != Action::SPLIT && action != Action::JOIN) {
        // if insert
        if (action == Action::INSERT && m_actions.top().col == col - 1) {
            m_actions.top().col = col;
            m_actions.top().text += ch;
        }
        
        // if delete
        else if (action == Action::DELETE && m_actions.top().col == col) {
            m_actions.top().text += ch;
        }
        
        // if backspace
        else if (action == Action::DELETE && m_actions.top().col == col + 1) {
            m_actions.top().col = col;
            m_actions.top().text.insert(m_actions.top().text.begin(), ch);
        }
        
        // not consecutive
        else {
            undoInfo info;
            info.action = action;
            info.row = row;
            info.col = col;
            info.text += ch;
            m_actions.push(info);
        }
    }
            
    else {
        undoInfo info;
        info.action = action;
        info.row = row;
        info.col = col;
        info.text += ch;
        m_actions.push(info);
    }
}

StudentUndo::Action StudentUndo::get(int& row, int& col, int& count, std::string& text) {
    // if nothing to undo
    if (m_actions.empty()) {
        return Action::ERROR;
    }
    
    undoInfo info = m_actions.top();
    m_actions.pop();
    row = info.row;
    col = info.col;
    text = info.text;
    
    // if undoing insert
    if (info.action == Action::INSERT) {
        count = text.size();
        col -= count;
        text.clear();
        return Action::DELETE;
    }
    
    // if undoing delete or backspace
    if (info.action == Action::DELETE) {
        count = 1;
        return Action::INSERT;
    }
    
    // if undoing line break
    if (info.action == Action::SPLIT) {
        count = 1;
        text.clear();
        return Action::JOIN;
    }
    
    // if undoing join
    if (info.action == Action::JOIN) {
        count = 1;
        text.clear();
        return Action::SPLIT;
    }
    
    return Action::ERROR;
}

void StudentUndo::clear() {
    while (!m_actions.empty()) {
        m_actions.pop();
    }
}
