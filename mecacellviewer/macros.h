#ifndef MVIEWER_MACROS_H
#define MVIEWER_MACROS_H
#include <map>
#include <functional>
#include <QString>

#define MECACELL_VIEWER_ADDITIONS(scenarClass)                                                                         \
    std::map<QString, std::map<QString, std::function<void(MecacellViewer::Renderer<scenarClass>*)> > > MCV_buttonMap; \
    void MCV_InitializeInterfaceAdditions()

#define ADD_BUTTON(label, menu, callback) MCV_buttonMap[#menu][label] = callback;

#define DEFINE_MEMBER_CHECKER(member)                                                         \
    template <typename T, typename V = bool>                                                  \
    struct has_##member : false_type {                                                        \
    };                                                                                        \
    template <typename T>                                                                     \
    struct has_##member<T,                                                                    \
	typename enable_if<!is_same<decltype(declval<T>().member), void>::value, bool>::type> \
	: true_type {                                                                         \
    };

#define HAS_MEMBER(C, member) has_##member<C>::value
#endif
