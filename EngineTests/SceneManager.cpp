#include "catch.hpp"
#include "SceneManager.h"

SCENARIO("SceneManager creating new scene", "[SceneManager]") {

	THEN("blargh") {
		auto sm = SceneManager();

		SceneCreateInfo createInfo;

		auto scene = sm.CreateScene(&createInfo);
	}
}