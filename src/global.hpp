#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include <list>
#include <map>
#include <fstream>
#include <set>
#include <algorithm>

enum State
{
    state_start,
    state_id,
    state_num,
    state_str,
    state_char,
    state_equal,
    state_end,
};