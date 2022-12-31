#pragma once
#include "../imgui/imgui.h"

static int tab = 0;

char buffer1[255]{};
char buffer2[255]{};

const char* login = "test";
const char* pass = "test";

static bool loggedIn = false;

static ImVec4 text = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);