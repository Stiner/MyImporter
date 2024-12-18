#pragma once

#define PMX_SAFE_DELETE(_exp_)       { if (_exp_ != nullptr) delete   _exp_; _exp_ = nullptr; }
#define PMX_SAFE_DELETE_ARRAY(_exp_) { if (_exp_ != nullptr) delete[] _exp_; _exp_ = nullptr; }
