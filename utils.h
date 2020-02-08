#pragma once

#define LIMIT(val,min,max) ((val) < (min)) ? (min) : (((max) < (val)) ? (max) : (val))