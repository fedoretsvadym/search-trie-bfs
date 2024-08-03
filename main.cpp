#include <bits/stdc++.h>

using namespace std;

class KeyPredictor {
public:
  KeyPredictor(string layout_type = "QWERTY", double starting_value = 5.0, double coeff = 0.25, int max_dstnc = 7) {
    hit_value = starting_value;
    coefficient = coeff;
    max_distance = max_dstnc;
    if (layout_type == "QWERTY") {
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

    while (!cur_stage.empty()) {
      char current_vertex = cur_stage.front();
      cur_stage.pop();
      for (char it : connections[current_vertex]) {
	if (distance.find(it) == distance.end()) {
	  distance[it] = distance[current_vertex] + 1;
	  if (distance[it] < max_distance) { // keys that located more than in max_distance keys from pressed have too low possibility
	    cur_stage.push(it);
	  }
	}
      }
    }

    vector<pair<char, double>> possibilities;
    for (const auto& it : distance) {
      if (possible_chars.find(it.first) != possible_chars.end()) {
	possibilities.push_back({it.first, hit_value * pow(coefficient, it.second)});
      }
    }
    return possibilities;
  }
  
private:
  unordered_map<char, unordered_set<char>> connections;
  double hit_value, coefficient;
  int max_distance;
};


class WordPredictor {
public:
  WordPredictor(string layout_type = "QWERTY", double starting_value = 5.0, double coeff = 0.25, int max_dstnc = 7) {
    root = new TrieNode();
    key_predictor = KeyPredictor(layout_type, starting_value, coeff, max_dstnc);
  }

  ~WordPredictor() {
    clearTrie(root);
  }

  bool insert(string word) {
    TrieNode* current_node = root;
    for (char it : word) {
      if (current_node->connections.find(it) == current_node->connections.end()) {
	current_node->connections[it] = new TrieNode();
      }
      current_node = current_node->connections[it];
    }
    current_node->is_word = true;
    return true;
  }
  
  bool insert(vector<string> words) {
    bool result = true;
    for (const auto& it : words) {
      result &= insert(it);
    }
    return result;
  }

  vector<pair<string, double>> get_possibility(string word) {
    vector<pair<string, double>> possibilities;
    queue<PossibilityNode*> processing_queue;
    processing_queue.push(new PossibilityNode(root, "", 1));

    for (char key : word) {
      set<char> possible_chars;
      queue<PossibilityNode*> copy = processing_queue;
      while (!copy.empty()) {
	PossibilityNode* node = copy.front();
	copy.pop();
	if (node != nullptr && node->node != nullptr) {
	  for (const auto& char_pos : node->node->connections) {
	    possible_chars.insert(char_pos.first);
	  }
	}
      }
      vector<pair<char, double>> char_possibilities = key_predictor.get_possibility(possible_chars, key);
      int queue_start_len = processing_queue.size();
      for (int i = 0; i < queue_start_len; ++i) {
	PossibilityNode* cur = processing_queue.front();
	processing_queue.pop();
	if (cur != nullptr && cur->node != nullptr) {
	  if (cur->node->is_word) {
	    possibilities.push_back({cur->content, cur->possibility});
	  }
	  for (const auto& it : char_possibilities) {
	    if (cur->node->connections.find(it.first) != cur->node->connections.end()) {
	      processing_queue.push(new PossibilityNode(cur->node->connections[it.first], cur->content + it.first, cur->possibility * it.second));
	    }
	  }
	}
      }
    }

    while (!processing_queue.empty()) {
      PossibilityNode* cur = processing_queue.front();
      processing_queue.pop();
      if (cur != nullptr && cur->node != nullptr && cur->node->is_word) {
	possibilities.push_back({cur->content, cur->possibility});
      }
      for (const auto& char_node : cur->node->connections) {
	processing_queue.push(new PossibilityNode(char_node.second, cur->content + char_node.first, cur->possibility));
      }
      delete cur; // Clean up memory
    }

    return possibilities;
  }

private:
  struct TrieNode {
    TrieNode() : is_word(false) {}
    unordered_map<char, TrieNode*> connections;
    bool is_word;
  };

  struct PossibilityNode {
    PossibilityNode(TrieNode* tn, string s, double pos) : node(tn), content(s), possibility(pos) {}
    TrieNode* node;
    string content;
    double possibility;
  };

  TrieNode* root;
  KeyPredictor key_predictor;

  void clearTrie(TrieNode* node) {
    if (node == nullptr) return;
    for (auto& pair : node->connections) {
      clearTrie(pair.second);
    }
    delete node;
  }
};


class SentencePredictor {
public:
  SentencePredictor() {}

  ~SentencePredictor() {}

  bool insert(string sentence) {
    vector<string> words = to_words(sentence);
    TrieNode* current_node = root;
    for (const auto& word : words) {
      if (current_node->connections.find(word) == current_node->connections.end()) {
	current_node->connections[word] = new TrieNode();    
      }
      current_node = current_node->connections[word];
      word_predictor.insert(word);
    }
    current_node->is_sentence = true;
    return true;
  }

  bool insert(vector<string> sentences) {
    bool status = true;
    for (const auto& sentence : sentences) {
      status &= insert(sentence);
    }
    return status;
  }

  vector<pair<string, double>> get_possibility(string sentence) {
    vector<string> words = to_words(sentence);
    vector<pair<string, double>> possibilities;
    queue<PossibilityNode*> processing_queue;
    processing_queue.push(new PossibilityNode(root, "", 1));

    for (const auto& word : words) {
      set<string> possible_words;
      queue<PossibilityNode*> copy = processing_queue;
      while (!copy.empty()) {
	PossibilityNode* node = copy.front();
	copy.pop();
	if (node != nullptr && node->node != nullptr) {
	  for (const auto& word_pos : node->node->connections) {
	    possible_words.insert(word_pos.first);
	  }
	}
      }
      vector<pair<string, double>> word_possibilities = word_predictor.get_possibility(word);
      int queue_start_len = processing_queue.size();
      for (int i = 0; i < queue_start_len; ++i) {
	PossibilityNode* cur = processing_queue.front();
	processing_queue.pop();
	if (cur != nullptr && cur->node != nullptr) {
	  if (cur->node->is_sentence) {
	    possibilities.push_back({cur->content, cur->possibility});
	  }
	  for (const auto& it : word_possibilities) {
	    if (cur->node->connections.find(it.first) != cur->node->connections.end()) {
	      processing_queue.push(new PossibilityNode(cur->node->connections[it.first], cur->content + " " + it.first, cur->possibility * it.second));
	    }
	  }
	}
      }
    }

    while (!processing_queue.empty()) {
      PossibilityNode* cur = processing_queue.front();
      processing_queue.pop();
      if (cur != nullptr && cur->node != nullptr && cur->node->is_sentence) {
	possibilities.push_back({cur->content, cur->possibility});
      }
      for (const auto& char_node : cur->node->connections) {
	processing_queue.push(new PossibilityNode(char_node.second, cur->content + " " + char_node.first, cur->possibility));
      }
      delete cur; // Clean up memory
    }

    return possibilities;
  }
  
private:
  struct TrieNode {
    TrieNode() : is_sentence(false) {}
    unordered_map<string, TrieNode*> connections;
    bool is_sentence;
  };

  struct PossibilityNode {
    PossibilityNode(TrieNode* tn, string s, double pos) : node(tn), content(s), possibility(pos) {}
    TrieNode* node;
    string content;
    double possibility;
  };

  TrieNode* root = new TrieNode();
  WordPredictor word_predictor;

  void clearTrie(TrieNode* node) {
    if (node == nullptr) return;
    for (auto& pair : node->connections) {
      clearTrie(pair.second);
    }
    delete node;
  }

  vector<string> to_words(string sentence) {
    vector<string> words;
    istringstream iss(sentence);

    string word;
    while (iss >> word) {
      words.push_back(word);
    }
    return words;
  }
};


int main() {
  SentencePredictor pred;
  bool status = pred.insert(vector<string>({"HOW TO FIND FRIENDS", "HOW TO FIND KEYS", "HOW TO PLAY MARIO", "HELLO", "HOWDY", "QWEQWEQW"}));
  cout << "insert status : " << status << "\n";
  vector<pair<string, double>> a = pred.get_possibility("HOW TI FUM");
  for (const auto& it : a) {
    cout << it.first << " " << it.second << "\n";
  }
  return 0;
}
