#include <Geode/Geode.hpp>
#include <Geode/modify/CCObject.hpp>

using namespace geode::prelude;

std::string getClassName(CCObject* node) {
    #ifdef GEODE_IS_WINDOWS
        return typeid(*node).name() + 6;
    #else 
        std::string ret;

        int status = 0;
        auto demangle = abi::__cxa_demangle(typeid(*node).name(), 0, 0, &status);
        if (status == 0) {
            ret = demangle;
        }
        free(demangle);

        return ret;
    #endif
}

struct MidMetadata {
	std::string className;
	int maxIndex;
	int currentIndex;
	std::function<void()> method;

	bool operator== (const MidMetadata& m) const {
        return m.className == className && m.maxIndex == maxIndex;
    }

};

class MidManager {

	public:

	std::vector<MidMetadata> midMethods;

	static MidManager& get() {
		static MidManager mm;
		return mm;
	}

	void registerMethod(std::string className, std::function<void()> method, int index = 0) {
		MidMetadata mm{className, index, 0, method};
		midMethods.push_back(mm);
	}

	void checkAndInvoke(std::string className) {

		MidMetadata midMethodToRemove;
		bool removeQueued = false;

		for (MidMetadata& mm : midMethods) {
			if (mm.className == className) {
				if (mm.currentIndex == mm.maxIndex) {
					mm.method();
					midMethodToRemove = mm;
					removeQueued = true;
					break;
				}
				mm.currentIndex++;
			}
		}

		if (removeQueued) {
			midMethods.erase(std::remove(midMethods.begin(), midMethods.end(), midMethodToRemove), midMethods.end());
		}
	}
};

class $modify(MyCCObject, CCObject) {

    CCObject* autorelease() {
        auto ret = CCObject::autorelease();
		MidManager::get().checkAndInvoke(getClassName(this));
		return ret;
    }
};

#include <Geode/modify/MenuLayer.hpp>

class $modify(MenuLayer) {

	bool init() {

		int funny = 0;

		MidManager::get().registerMethod("MenuGameLayer", [&funny] {
			funny = 10;
		});

		auto ret = MenuLayer::init();

		log::info("funny: {}", funny);

		return ret;
	}
};
