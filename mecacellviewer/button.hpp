#ifndef BUTTON_HPP
#define BUTTON_HPP
#include <QString>
#include <QColor>
#include <functional>

enum class ButtonType { squared, rectangular, check_squared, check_rectangular };
template <typename RendererType> class Button {
	friend RendererType;

 private:
	ButtonType type = ButtonType::rectangular;
	QString name, menu, label;
	std::function<void(RendererType *, Button *)> onClicked;
	QColor color = QColor(255, 255, 255);
	bool checked = false;
	bool updt = true;  // needs to be updated

 public:
	Button() {}
	Button(QString n, QString m, QString l, decltype(onClicked) c,
	       ButtonType bt = ButtonType::rectangular, bool chk = false)
	    : name(n), menu(m), label(l), onClicked(c), type(bt), checked(chk){};

	QColor getColor() { return color; }
	bool getChecked() { return checked; }
	QString getName() { return name; }
	QString getMenu() { return menu; }
	QString getLabel() { return label; }
	void setLabel(QString l) { label = l; }
	void setMenu(QString m) { menu = m; }

	bool needsToBeUpdated() { return updt; }
	void updateOK() { updt = true; }
	void clicked(RendererType *r) {
		if (type == ButtonType::check_squared || type == ButtonType::check_rectangular) {
			checked = !checked;
			updt = true;
		}
		onClicked(r, this);
	}
	void setLabel(const QString &l) {
		label = l;
		updt = true;
	}
	void setOnClicked(decltype(onClicked) f) {
		onClicked = f;
		updt = true;
	}
	void setColor(const int r, const int g, const int b) {
		color = QColor(r, g, b);
		updt = true;
	}
	void setColor(const QColor &c) {
		color = c;
		updt = true;
	}
	void setColor(const double r, const double g, const double b) {
		color = QColor::fromRgbF(r, g, b);
		updt = true;
	}
};
#endif
