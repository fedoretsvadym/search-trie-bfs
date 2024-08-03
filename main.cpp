#include <bits/stdc++.h>

using namespace std;

class KeyPredictor {
 public:
  KeyPredictor() {
  }
  
  KeyPredictor(string layout_type) {
    if(layout_type == "QWERTY") {
      connections = {
	// Number row
	{'1', {'2'}},
	{'2', {'1', '3', 'Q', 'W'}},
	{'3', {'2', '4', 'W', 'E'}},
	{'4', {'3', '5', 'E', 'R'}},
	{'5', {'4', '6', 'R', 'T'}},
	{'6', {'5', '7', 'T', 'Y'}},
	{'7', {'6', '8', 'Y', 'U'}},
	{'8', {'7', '9', 'U', 'I'}},
	{'9', {'8', '0', 'I', 'O'}},
	{'0', {'9', 'O', 'P'}},

	// Top row
	{'Q', {'W', 'A', '2'}},
	{'W', {'Q', 'E', 'A', 'S', '2', '3'}},
	{'E', {'W', 'R', 'S', 'D', '3', '4'}},
	{'R', {'E', 'T', 'D', 'F', '4', '5'}},
	{'T', {'R', 'Y', 'F', 'G', '5', '6'}},
	{'Y', {'T', 'U', 'G', 'H', '6', '7'}},
	{'U', {'Y', 'I', 'H', 'J', '7', '8'}},
	{'I', {'U', 'O', 'J', 'K', '8', '9'}},
	{'O', {'I', 'P', 'K', 'L', '9', '0'}},
	{'P', {'O', 'L', '0'}},

	// Middle row
	{'A', {'S', 'Q', 'Z', 'X'}},
	{'S', {'A', 'D', 'W', 'X', 'Z', 'C'}},
	{'D', {'S', 'F', 'E', 'C', 'X', 'V'}},
	{'F', {'D', 'G', 'R', 'V', 'C', 'B'}},
	{'G', {'F', 'H', 'T', 'B', 'V', 'N'}},
	{'H', {'G', 'J', 'Y', 'N', 'B', 'M'}},
	{'J', {'H', 'K', 'U', 'M'}},
	{'K', {'J', 'L', 'I'}},
	{'L', {'K', 'O', 'P'}},

	// Bottom row
	{'Z', {'A', 'X'}},
	{'X', {'Z', 'C', 'S', 'V'}},
	{'C', {'X', 'V', 'D', 'B'}},
	{'V', {'C', 'B', 'F', 'N'}},
	{'B', {'V', 'N', 'G', 'M'}},
	{'N', {'B', 'M', 'H'}},
	{'M', {'N', 'J', 'K'}}
      };
    }
    else {
      throw runtime_error("failed to construct");
    }
  }
  
  vector<pair<char, double>> get_possibility(set<char> possible_chars, char given_char) {
    queue<char> cur_stage;
    cur_stage.push(given_char);
    map<char, int> distance;
    distance[given_char] = 0;
    while(!cur_stage.empty()) {
      char current_vertex = cur_stage.front();
      cur_stage.pop();
      for(auto it:connections[current_vertex]) {
	if(distance.find(it) == distance.end()) {
	  distance[it] = distance[current_vertex] + 1;
	  if(distance[it] < 7) { // keys that located more than in 7 keys from pressed have too low possibility
	    cur_stage.push(it);
	  }
	}
      }
    }
    vector<pair<char, double>> possibilities;
    for(auto it:distance) {
      if(possible_chars.find(it.first) != possible_chars.end()) {
	possibilities.push_back({it.first, 5.0*pow(0.25, it.second)});
      }
    }
    return possibilities;
  }
  
 private:
  unordered_map<char, unordered_set<char> > connections;  
};


class WordPredictor {
public:
  WordPredictor(string layout_type) {
    root = new TrieNode();
    key_predictor = KeyPredictor(layout_type);
  }

  bool insert(string word) {
    TrieNode* current_node = root;
    for(auto it:word) {
      if(current_node->connections[it] == NULL) {
	current_node->connections[it] = new TrieNode();	
      }
      current_node = current_node->connections[it];
    }
    current_node->is_word = true;
    return true;
  }
  
  bool insert(vector<string> words) {
    bool result = true;
    for(auto it:words) {
      result &= insert(it);
    }
    return result;
  }

  vector<pair<string, double> > get_possibility(string users_word) {
    vector<pair<string, double>> possibilities;
    TrieNode* current_node = root;
    queue<pair<TrieNode*, pair<double, string> > > processing_queue;
    processing_queue.push({root, {1, ""}});
    for(auto key:users_word) {
      set<char> possible_chars;
      queue<pair<TrieNode*, pair<double, string> > > copy = processing_queue;
      while(!copy.empty()) {	
	for(auto pair : copy.front().first->connections) {
	  possible_chars.insert(pair.first);
	}
	copy.pop();
      }
      vector<pair<char, double> > predicted = key_predictor.get_possibility(possible_chars, key);
      int len = processing_queue.size();
      for(int i = 0; i < len; i++) {
	pair<TrieNode*, pair<double, string> > cur = processing_queue.front();
	processing_queue.pop();
	TrieNode* current_node = cur.first;
	double possibility = cur.second.first;
	string s = cur.second.second;
	if(current_node->is_word) {
	  possibilities.push_back({s, possibility});
	}
	for(auto it:predicted) {
	  if(current_node->connections[it.first] != NULL) {
	    processing_queue.push({current_node->connections[it.first], {possibility * it.second, s + it.first}});
	  }
	}
      }
    }
    while(!processing_queue.empty()) {
      auto current = processing_queue.front();
      if(current.first->is_word) {
	possibilities.push_back({current.second.second, current.second.first});
      }
      for(auto pair : current.first->connections) {
	processing_queue.push({pair.second, {current.second.first, current.second.second + pair.first}});
      }
      processing_queue.pop();
    }
    return possibilities;
  }

private:
  struct TrieNode {
    TrieNode() {
      is_word = false;
    }
    unordered_map<char, TrieNode*> connections;
    bool is_word;
  };

  TrieNode* root;
  KeyPredictor key_predictor;
};


int main() {
  WordPredictor pred("QWERTY");
  bool status = pred.insert(vector<string>({"ASD", "BSC", "QW", "QWE", "QWD", "POQ", "SWY", "QWEG", "QWEAS"}));
  cout << "insert status : " << status << "\n";
  vector<pair<string, double> > a = pred.get_possibility("QWE");
  for(auto it:a) {
    cout << it.first << " " << it.second << "\n";
  }
  return 0;
}
