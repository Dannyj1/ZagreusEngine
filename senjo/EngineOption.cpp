//-----------------------------------------------------------------------------
// Copyright (c) 2015-2019 Shawn Chidester <zd3nik@gmail.com>
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "EngineOption.h"

#include <utility>

namespace senjo {

static const char* optButtonName = "button";
static const char* optCheckName = "check";
static const char* optComboName = "combo";
static const char* optSpinName = "spin";
static const char* optStringName = "string";
static const char* optUnknownName = "unknown";

//-----------------------------------------------------------------------------
    EngineOption::OptionType EngineOption::toOptionType(const std::string &name) {
      if (!iEqual(name, optButtonName)) {
        return Button;
      }
      if (!iEqual(name, optCheckName)) {
        return Checkbox;
      }
      if (!iEqual(name, optComboName)) {
        return ComboBox;
      }
      if (!iEqual(name, optSpinName)) {
        return Spin;
      }
      if (!iEqual(name, optStringName)) {
        return String;
      }
        return Unknown;
    }

//-----------------------------------------------------------------------------
    std::string EngineOption::getTypeName(const OptionType type) {
        switch (type) {
            case Button:
              return optButtonName;
            case Checkbox:
              return optCheckName;
            case ComboBox:
              return optComboName;
            case Spin:
              return optSpinName;
            case String:
              return optStringName;
            default:
                break;
        }
        return optUnknownName;
    }

//-----------------------------------------------------------------------------
    EngineOption::EngineOption(std::string optName,
                               const std::string& defaultValue,
                               const OptionType optType, const int64_t minValue,
                               const int64_t maxValue,
                               const std::set<std::string>& comboValues)
        : optType(optType),
          optName(std::move(optName)),
          optValue(defaultValue),
          defaultValue(defaultValue),
          minValue(minValue),
          maxValue(maxValue),
          comboValues(comboValues) {}

    //-----------------------------------------------------------------------------
    int64_t EngineOption::getIntValue() const {
        return toNumber<int64_t>(optValue);
    }

//-----------------------------------------------------------------------------
    int64_t EngineOption::getDefaultIntValue() const {
        return toNumber<int64_t>(defaultValue);
    }

//-----------------------------------------------------------------------------
    std::set<int64_t> EngineOption::getIntComboValues() const {
        std::set<int64_t> values;
        for (const auto& value : comboValues) {
          auto n = toNumber<int64_t>(value, -1);
          if (n >= 0) {
            values.insert(n);
          }
        }
        return values;
    }

//-----------------------------------------------------------------------------
    bool EngineOption::setValue(const std::string &value) {
        switch (optType) {
            case Checkbox:
                if (!iEqual(value, "true") && !iEqual(value, "false")) {
                    return false;
                }
                break;
            case ComboBox:
                if (!comboValues.count(value)) {
                    return false;
                }
                break;
            case Spin: {
              auto intval = toNumber<int64_t>(value, minValue - 1);
              if (intval < minValue || intval > maxValue) {
                return false;
                }
                break;
            }
            case String:
                break;
            default:
                return false;
        }
        optValue = value;
        return true;
    }

//-----------------------------------------------------------------------------
    bool EngineOption::setValue(const int64_t value) {
        return setValue(std::to_string(value));
    }

//-----------------------------------------------------------------------------
    void EngineOption::setDefaultValue(const int64_t value) {
        setDefaultValue(std::to_string(value));
    }

//-----------------------------------------------------------------------------
    void EngineOption::setComboValues(const std::set<int64_t> &values) {
        comboValues.clear();
        for (auto value : values) {
            comboValues.insert(std::to_string(value));
        }
    }

} // namespace senjo
