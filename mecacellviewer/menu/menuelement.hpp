#ifndef MENUELEMENT_HPP
#define MENUELEMENT_HPP
#include <QString>
#include <cassert>

enum class elementType { group, checkable, exclusiveGroup };
template <typename R> struct MenuElement {
	QString name;
	elementType type = elementType::checkable;
	bool currentChecked = true;
	bool previousChecked = true;
	vector<MenuElement> elems;
	std::function<void(R*, MenuElement*)> onToggled;
	void add(const MenuElement& e) { elems.push_back(e); }
	int count(const QString& n) {
		for (auto& e : elems) {
			if (e.name == n) return 1;
		}
		return 0;
	}
	MenuElement<R>& at(const QString& n) {
		qDebug() << name << " trying to access " << n;
		assert(count(n));
		for (auto& e : elems) {
			if (e.name == n) return e;
		}
		throw std::runtime_error("menu element not found");
	}
	bool isChecked() { return currentChecked; }

	static QString typeToString(elementType t) {
		switch (t) {
			case elementType::exclusiveGroup:
				return "exclusiveGroup";
			case elementType::group:
				return "group";
			default:
			case elementType::checkable:
				return "checkable";
		}
	}
	static QString typeToSymbol(elementType t) {
		switch (t) {
			case elementType::group:
				return "__";
			case elementType::checkable:
				return "☑ ";
			default:
				return "⊙ ";
		}
	}
	MenuElement<R>* atptr(const QString& s) {
		qDebug() << name << " looking for " << s;
		assert(count(s));
		for (auto& e : elems) {
			if (e.name == s) return &e;
		}
		return nullptr;
	}

	void callAll(R* r) {
		if (onToggled) onToggled(r, this);
		for (auto& e : elems) e.callAll(r);
	}

	void restoreMeAndMyChildrens() {
		setChecked(previousChecked);
		for (auto& e : elems) e.restoreMeAndMyChildrens();
	}
	void uncheckMeAndMyChildrens() {
		previousChecked = currentChecked;
		currentChecked = false;
		for (auto& e : elems) e.uncheckMeAndMyChildrens();
	}

	void setChecked(bool c) {
		previousChecked = c;
		currentChecked = c;
	}

	void restoreChildrenChecked() {
		for (auto& e : elems) e.restoreMeAndMyChildrens();
	}
	void updateCheckedFromList(R*, const vector<pair<QList<QVariant>, bool>>& v) {
		for (auto& var : v) {
			MenuElement<R>* nxt = this;
			for (auto& e : var.first) {
				if (e.toString() != name) {
					// on parcours la liste
					nxt = nxt->atptr(e.toString());
				}
			}
			if (var.second) {
				nxt->setChecked(true);
				nxt->restoreChildrenChecked();
			} else {
				nxt->uncheckMeAndMyChildrens();
			}
		}
	}

	QString toJSON() {
		QString res = "{\"name\":\"" + name + "\",\"type\":\"" + typeToString(type) +
		              "\", \"checked\":" + (currentChecked ? "true" : "false") +
		              ",\"elems\":[";
		for (size_t i = 0; i < elems.size(); ++i) {
			res += elems[i].toJSON();
			if (i < elems.size() - 1) {
				res += ",";
			}
		}
		res += "]}";
		return res;
	}
	void print(int padding = 0) { print(padding, type); }
	void print(int padding, elementType currentType) {
		for (int i = 0; i < padding; ++i) std::cerr << " ";
		std::cerr << typeToSymbol(type == elementType::exclusiveGroup ? elementType::group :
		                                                                currentType)
		                 .toStdString()
		          << " " << name.toStdString() << endl;
		for (auto& e : elems) {
			if (type == elementType::exclusiveGroup) {
				e.print(padding + 2, elementType::exclusiveGroup);
			} else
				e.print(padding + 2);
		}
	}

	MenuElement() {}
	MenuElement(QString n) : name(std::move(n)) {}
	template <typename N>
	MenuElement(N&& n, elementType t) : name(std::forward<N>(n)), type(t) {}
	template <typename N>
	MenuElement(N&& n, bool c)
	    : name(std::forward<N>(n)), currentChecked(c), previousChecked(c) {}

	template <typename N>
	MenuElement(N&& n, std::vector<MenuElement> v)
	    : name(std::forward<N>(n)), elems(std::move(v)) {}
	template <typename N, typename T>
	MenuElement(N&& n, T&& t, std::vector<MenuElement> v)
	    : name(std::forward<N>(n)), type(std::forward<T>(t)), elems(std::move(v)) {}
};

#endif
