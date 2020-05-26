#ifndef PGCONF_HPP
#define PGCONF_HPP

#include <string>
#include <memory>
#include <iostream>
#include <fstream>
#include <vector>

using std::string;
using std::shared_ptr;
using std::vector;
using std::cout;
using std::endl;

namespace PGConf {

	class Conf {
	protected:
		void skipSpaces(std::istream &is) {
			while (is.peek() != EOF) {
				char c = is.get();
				if (c == ' ' || c == '\t' || c == '\n' || c == '\r') continue;
				is.unget();
				return;
			}
		}

		void seekToLineEnd(std::istream &is) {
			while (is.peek() != EOF) {
				char c = is.get();
				if (c == '\n') return;
			}
		}

	public:
		virtual void read(std::istream &is) = 0;
		virtual void write(std::ostream &os) = 0;
	};

	// tokens is a single string
	class Token : public Conf {
		vector<char> _buf;
	public:
		void read(std::istream &is) override {
			skipSpaces(is);
			while (is.peek() != EOF) {
				char c = is.get();
				if (c == '#') {
					seekToLineEnd(is); // skip comments
					return;
				}
				if (c == ' ' || c == '\n') {
					return;
				}
				if (c == '\r') {
					char next = is.get();
					if (next != '\n') {
						// swallow the new line character
						is.unget();
					}
					return;
				}
				if (c == '=' || c == '{' || c == '}') {
					is.unget();
					return;
				}
				_buf.emplace_back(c);
			}
		}
		void write(std::ostream &os) override {
			for (char c : _buf) {
				os.put(c);
			}
		}

		bool isEmpty() { return _buf.empty(); }
	};


	// k-v pairs
	class KVPair : public Conf {
		shared_ptr<Token> _k, _v;

	public:
		KVPair(const shared_ptr<Token> &key) {
			_k = key;
		}

		void read(std::istream &is) override {
			_v = std::make_shared<Token>();
			_v->read(is);
		}

		void write(std::ostream &os) override {
			_k->write(os);
			os.write(" = ", 3);
			_v->write(os);
		}
	};


	// scopes
	class Scope : public Conf {
		shared_ptr<Token> _k;
		vector<shared_ptr<Conf>> _sub;

		int _depth;

	public:
		Scope(const shared_ptr<Token> &key, int depth) {
			_k = key;
			_depth = depth;
		}

		void read(std::istream &is) override {
			skipSpaces(is);
			char lb = is.get();
			if (lb != '{') throw;
			while (true) {
				skipSpaces(is);
				char rb = is.get();
				if (rb == '}') break;
				is.unget();
				auto keyToken = std::make_shared<Token>();
				keyToken->read(is);
				if (keyToken->isEmpty()) continue;
				skipSpaces(is);
				char eq = is.get();
				if (eq == '=') {
					// k-v pairs or scopes
					skipSpaces(is);
					char next = is.peek();
					if (next == '{') {
						// scopes
						auto scope = std::make_shared<Scope>(keyToken, _depth + 1);
						scope->read(is);
						_sub.emplace_back(scope);
					} else {
						// k-v pairs
						auto pair = std::make_shared<KVPair>(keyToken);
						pair->read(is);
						_sub.emplace_back(pair);
					}
				} else {
					// single token values
					is.unget();
					_sub.emplace_back(keyToken);
					continue;
				}
			}
		}

		void write(std::ostream &os) override {
			_k->write(os);
			os.write(" = {\n", 5);
			for (const auto &sub : _sub) {
				for (int i = 0; i < _depth; ++i) {
					os.put('\t');
				}
				sub->write(os);
				os.put('\n');
			}
			for (int i = 1; i < _depth; ++i) {
				os.put('\t');
			}
			os.write("}", 1);
		}

	};

	class ConfFile : public Conf {
	public:
		vector<shared_ptr<Conf>> contents;
		void read(std::istream &is) override {
			while (is.peek() != EOF) {
				auto keyToken = std::make_shared<Token>();
				keyToken->read(is);
				if (keyToken->isEmpty()) return;
				skipSpaces(is);
				char eq = is.get();
				if (eq != '=') throw; // top-level confs should all be k-v pairs or scopes
				skipSpaces(is);
				char next = is.peek();
				if (next == '{') {
					// scopes
					auto scope = std::make_shared<Scope>(keyToken, 1);
					scope->read(is);
					contents.emplace_back(scope);
				} else {
					// k-v pairs
					auto pair = std::make_shared<KVPair>(keyToken);
					pair->read(is);
					contents.emplace_back(pair);
				}
			}
		}

		void write(std::ostream &os) override {
			for (const auto &content : contents) {
				content->write(os);
				os.put('\n');
			}
		}

		bool readFromFile(const string &filePath) {
			std::ifstream ifs(filePath, std::ios_base::binary);
			if (!ifs.good()) {
				cout << "´ò¿ª\"" + filePath + "\"Ê§°Ü£¡" << endl;
				return false;
			}
			try {
				this->read(ifs);
				return true;
			} catch (...) {
				cout << "¶ÁÈ¡\"" + filePath + "\"Ê§°Ü£¡" << endl;
				return false;
			}
		}

		bool writeToFile(const string &filePath) {
			std::ofstream ofs(filePath, std::ios_base::binary);
			if (!ofs.good()) {
				cout << "´ò¿ª\"" + filePath + "\"Ê§°Ü£¡" << endl;
				return false;
			}
			try {
				this->write(ofs);
				return true;
			} catch (...) {
				cout << "Ð´Èë\"" + filePath + "\"Ê§°Ü£¡" << endl;
				return false;
			}
		}
	};

}

#endif
