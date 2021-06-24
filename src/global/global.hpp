#pragma once
#include <iostream>
#include <memory>
#include <vector>
#include <list>
#include <map>
#include <fstream>
#include <ostream>
#include <set>
#include <algorithm>
#include <limits>

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