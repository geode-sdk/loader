#pragma warning(disable: 4067)

#include <Setting.hpp>
#include <InternalMod.hpp>
#include <utils/string.hpp>
#include <utils/general.hpp>
#include <utils/convert.hpp>

USE_GEODE_NAMESPACE();

bool StringSetting::replaceWithBuiltInFilter(std::string& filter) {
	switch (hash(string_utils::toLower(filter).c_str())) {
		case hash("anything"):
			filter = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789_-./\\:;,+()[]{}&%$#\"'?!@§^*<>|=~ ";
			return true;

		case hash("lowercase"):
			filter = "abcdefghijklmnopqrstuvwxyz";
			return true;
			
		case hash("letters"):
			filter = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ";
			return true;

		case hash("normal"):
			filter = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789 ";
			return true;

		case hash("password"):
			filter = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789-";
			return true;

		case hash("username"):
			filter = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
			return true;

		case hash("comment"):
			filter = " abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789.,-!?:;)(/\\\"'`*=+-_%[]<>|@&^#{}%$~";
			return true;
	}
	return false;
}

Result<ccColor3B> ColorSetting::parseColor(nlohmann::json const& json) {
	if (json.is_array()) {
		if (json.size() == 3) {
			if (
				json[0].is_number_integer() &&
				json[1].is_number_integer() &&
				json[2].is_number_integer()
			) {
				auto r = json[0].get<int>();
				auto g = json[1].get<int>();
				auto b = json[2].get<int>();
				if (
					0 <= r && r <= 255 &&
					0 <= g && g <= 255 &&
					0 <= b && b <= 255
				) {
					return Ok<ccColor3B>({
						static_cast<GLubyte>(r),
						static_cast<GLubyte>(g),
						static_cast<GLubyte>(b),
					});
				} else {
					return Err<>("Color is an array whose elements are not all within [0, 255]");
				}
			} else {
				return Err<>("Color is an array whose elements are not all ints");
			}
		} else {
			return Err<>("Color is an array that is not 3 elements (R, G, B) in size");
		}
	}

	if (json.is_string()) {
		std::string str = json;
		if (string_utils::contains(str, ",")) {
			auto colors = string_utils::split(str, ",");
			if (colors.size() == 3) {
				try {
					auto r = std::stoi(colors[0]);
					auto g = std::stoi(colors[1]);
					auto b = std::stoi(colors[2]);
					if (
						0 <= r && r <= 255 &&
						0 <= g && g <= 255 &&
						0 <= b && b <= 255
					) {
						return Ok<ccColor3B>({
							static_cast<GLubyte>(r),
							static_cast<GLubyte>(g),
							static_cast<GLubyte>(b),
						});
					} else {
						return Err<>("Color is an RGB string whose elements are not all within [0, 255]");
					}
				} catch(...) {
					return Err<>("Color is an RGB string whose elements are not all ints");
				}
			} else {
				return Err<>("Color is a string that is not in the format \"r,g,b\" or a hex number");
			}
		} else {
			if (str[0] == '#') {
				str = str.substr(1);
			}
			if (str.size() > 6) {
				return Err<>("Color is too long for a hex string");
			}
			try {
				auto color = std::stoi(str, nullptr, 16);
				return Ok<>(cc3x(color));
			} catch(...) {
				return Err<>("Color is not a valid hex string");
			}
		}
	}

	if (json.is_object()) {
		if (
			json.contains("r") &&
			json.contains("g") &&
			json.contains("b")
		) {
			if (
				json["r"].is_number_integer() &&
				json["g"].is_number_integer() &&
				json["b"].is_number_integer()
			) {
				auto r = json["r"].get<int>();
				auto g = json["g"].get<int>();
				auto b = json["b"].get<int>();
				if (
					0 <= r && r <= 255 &&
					0 <= g && g <= 255 &&
					0 <= b && b <= 255
				) {
					return Ok<ccColor3B>({
						static_cast<GLubyte>(r),
						static_cast<GLubyte>(g),
						static_cast<GLubyte>(b),
					});
				} else {
					return Err<>("Color is an object whose elements are not all within [0, 255]");
				}
			} else {
				return Err<>("Color is an object whose elements are not all ints");
			}
		} else {
			return Err<>("Color is an object which lacks one or all of r,g,b");
		}
	}

	return Err<>("Color is neither an array, a string nor an object");
}

Result<ccColor4B> ColorAlphaSetting::parseColor(nlohmann::json const& json) {
	if (json.is_array()) {
		if (json.size() == 4) {
			if (
				json[0].is_number_integer() &&
				json[1].is_number_integer() &&
				json[2].is_number_integer() &&
				json[3].is_number_integer()
			) {
				auto r = json[0].get<int>();
				auto g = json[1].get<int>();
				auto b = json[2].get<int>();
				auto a = json[3].get<int>();
				if (
					0 <= r && r <= 255 &&
					0 <= g && g <= 255 &&
					0 <= b && b <= 255 &&
					0 <= a && a <= 255
				) {
					return Ok<ccColor4B>({
						static_cast<GLubyte>(r),
						static_cast<GLubyte>(g),
						static_cast<GLubyte>(b),
						static_cast<GLubyte>(a),
					});
				} else {
					return Err<>("Color is an array whose elements are not all within [0, 255]");
				}
			} else {
				return Err<>("Color is an array whose elements are not all ints");
			}
		} else {
			return Err<>("Color is an array that is not 4 elements (R, G, B, A) in size");
		}
	}

	if (json.is_string()) {
		std::string str = json;
		if (string_utils::contains(str, ",")) {
			auto colors = string_utils::split(str, ",");
			if (colors.size() == 4) {
				try {
					auto r = std::stoi(colors[0]);
					auto g = std::stoi(colors[1]);
					auto b = std::stoi(colors[2]);
					auto a = std::stoi(colors[3]);
					if (
						0 <= r && r <= 255 &&
						0 <= g && g <= 255 &&
						0 <= b && b <= 255 &&
						0 <= a && a <= 255
					) {
						return Ok<ccColor4B>({
							static_cast<GLubyte>(r),
							static_cast<GLubyte>(g),
							static_cast<GLubyte>(b),
							static_cast<GLubyte>(a),
						});
					} else {
						return Err<>("Color is an RGB string whose elements are not all within [0, 255]");
					}
				} catch(...) {
					return Err<>("Color is an RGB string whose elements are not all ints");
				}
			} else {
				return Err<>("Color is a string that is not in the format \"r,g,b,a\"");
			}
		} else {
			return Err<>("Color is not an \"r,g,b,a\" string. Note that hex strings are not supported for RGBA");
		}
	}

	if (json.is_object()) {
		if (
			json.contains("r") &&
			json.contains("g") &&
			json.contains("b") &&
			json.contains("a")
		) {
			if (
				json["r"].is_number_integer() &&
				json["g"].is_number_integer() &&
				json["b"].is_number_integer() &&
				json["a"].is_number_integer()
			) {
				auto r = json["r"].get<int>();
				auto g = json["g"].get<int>();
				auto b = json["b"].get<int>();
				auto a = json["a"].get<int>();
				if (
					0 <= r && r <= 255 &&
					0 <= g && g <= 255 &&
					0 <= b && b <= 255 &&
					0 <= a && a <= 255
				) {
					return Ok<ccColor4B>({
						static_cast<GLubyte>(r),
						static_cast<GLubyte>(g),
						static_cast<GLubyte>(b),
						static_cast<GLubyte>(a),
					});
				} else {
					return Err<>("Color is an object whose elements are not all within [0, 255]");
				}
			} else {
				return Err<>("Color is an object whose elements are not all ints");
			}
		} else {
			return Err<>("Color is an object which lacks one or all of r,g,b,a");
		}
	}

	return Err<>("Color is neither an array, a string nor an object");
}

template <>
Result<BoolSetting*> GeodeSetting<BoolSetting>::parse(nlohmann::json const& json) {
	auto res = new BoolSetting;
	res->parseFields(json);
	bool foundDefaultValue = false;
	if (json.contains("value")) {
		auto value = json["value"];
		if (value.is_object()) {
			if (value.contains("default")) {
				if (value["default"].is_boolean()) {
					res->m_default = value["default"];
					foundDefaultValue = true;
				} else {
					delete res;
					return Err<>("Setting has \"value.default\" but it is not a boolean");
				}
			} else {
				InternalMod::get()->throwError(
					"Boolean setting has \"value\", but no \"value.default\"",
					Severity::Warning
				);
			}
		} else {
			delete res;
			return Err<>("Setting has \"value\" but it is not an object");
		}
	}
	if (json.contains("default")) {
		if (foundDefaultValue) {
			InternalMod::get()->throwError(
				"Boolean setting has both \"value.default\" and \"default\"; "
				"using \"value.default\"",
				Severity::Warning
			);
		} else {
			if (json["default"].is_boolean()) {
				res->m_default = json["default"];
				foundDefaultValue = true;
			} else {
				delete res;
				return Err<>("Setting has \"default\" but it is not a boolean");
			}
		}
	}
	if (!foundDefaultValue) {
		delete res;
		return Err<>("Setting has neither \"default\" nor \"value.default\"");
	}
	return Ok<>(res);
}

template<>
Result<IntSetting*> GeodeSetting<IntSetting>::parse(nlohmann::json const& json) {
	auto res = new IntSetting;
	res->parseFields(json);
	if (json.contains("value")) {
		auto value = json["value"];
		if (value.is_object()) {
			if (value.contains("default")) {
				if (value["default"].is_number_integer()) {
					res->m_default = value["default"];
				} else {
					delete res;
					return Err<>("Setting has \"value.default\" but it is not an integer");
				}
			} else {
				delete res;
				return Err<>("Setting has \"value\" but no \"value.default\"");
			}

			if (value.contains("min")) {
				if (value["min"].is_number_integer()) {
					res->m_min = value["min"];
				} else {
					delete res;
					return Err<>("Setting has \"value.min\" but it is not an int");
				}
			}

			if (value.contains("max")) {
				if (value["max"].is_number_integer()) {
					res->m_max = value["max"];
				} else {
					delete res;
					return Err<>("Setting has \"value.max\" but it is not an int");
				}
			}
		} else {
			delete res;
			return Err<>("Setting has \"value\" but it is not an object");
		}
	} else {
		delete res;
		return Err<>("Setting lacks \"value\"");
	}
	if (json.contains("control")) {
		auto control = json["control"];
		if (control.is_object()) {
			if (control.contains("slider")) {
				if (control["slider"].is_boolean()) {
					res->m_slider = control["slider"];
				} else {
					delete res;
					return Err<>("Setting has \"control.slider\" but it is not a boolean");
				}
			}
			if (control.contains("input")) {
				if (control["input"].is_boolean()) {
					res->m_input = control["input"];
				} else {
					delete res;
					return Err<>("Setting has \"control.input\" but it is not a boolean");
				}
			}
			if (control.contains("arrows")) {
				if (control["arrows"].is_boolean()) {
					res->m_arrows = control["arrows"];
				} else {
					delete res;
					return Err<>("Setting has \"control.arrows\" but it is not a boolean");
				}
			}
		} else {
			delete res;
			return Err<>("Setting has \"control\" but it is not an object");
		}
	}
	return Ok<>(res);
}

template<>
Result<FloatSetting*> GeodeSetting<FloatSetting>::parse(nlohmann::json const& json) {
	auto res = new FloatSetting;
	res->parseFields(json);
	if (json.contains("value")) {
		auto value = json["value"];
		if (value.is_object()) {
			if (value.contains("default")) {
				if (value["default"].is_number()) {
					res->m_default = value["default"];
				} else {
					delete res;
					return Err<>("Setting has \"value.default\" but it is not a float");
				}
			} else {
				delete res;
				return Err<>("Setting has \"value\" but no \"value.default\"");
			}

			if (value.contains("min")) {
				if (value["min"].is_number()) {
					res->m_min = value["min"];
				} else {
					delete res;
					return Err<>("Setting has \"value.min\" but it is not a float");
				}
			}

			if (value.contains("max")) {
				if (value["max"].is_number()) {
					res->m_max = value["max"];
				} else {
					delete res;
					return Err<>("Setting has \"value.max\" but it is not a float");
				}
			}
		} else {
			delete res;
			return Err<>("Setting has \"value\" but it is not an object");
		}
	} else {
		delete res;
		return Err<>("Setting lacks \"value\"");
	}
	if (json.contains("control")) {
		auto control = json["control"];
		if (control.is_object()) {
			if (control.contains("slider")) {
				if (control["slider"].is_boolean()) {
					res->m_slider = control["slider"];
				} else {
					delete res;
					return Err<>("Setting has \"control.slider\" but it is not a boolean");
				}
			}
			if (control.contains("input")) {
				if (control["input"].is_boolean()) {
					res->m_input = control["input"];
				} else {
					delete res;
					return Err<>("Setting has \"control.input\" but it is not a boolean");
				}
			}
			if (control.contains("arrows")) {
				if (control["arrows"].is_boolean()) {
					res->m_arrows = control["arrows"];
				} else {
					delete res;
					return Err<>("Setting has \"control.arrows\" but it is not a boolean");
				}
			}
		} else {
			delete res;
			return Err<>("Setting has \"control\" but it is not an object");
		}
	}
	return Ok<>(res);
}

template<>
Result<StringSetting*> GeodeSetting<StringSetting>::parse(nlohmann::json const& json) {
	auto res = new StringSetting;
	res->parseFields(json);
	if (json.contains("value")) {
		auto value = json["value"];
		if (value.is_object()) {
			if (value.contains("default")) {
				if (value["default"].is_string()) {
					res->m_default = value["default"];
				} else {
					delete res;
					return Err<>("Setting has \"value.default\" but it is not a string");
				}
			} else {
				delete res;
				return Err<>("Setting has \"value\" but no \"value.default\"");
			}
			if (value.contains("filter")) {
				if (value["filter"].is_string()) {
					res->m_filter = value["filter"];
					res->replaceWithBuiltInFilter(res->m_filter);
				} else {
					delete res;
					return Err<>("Setting has \"value.filter\" but it is not a string");
				}
			}
		} else {
			delete res;
			return Err<>("Setting has \"value\" but it is not an object");
		}
	} else {
		delete res;
		return Err<>("Setting lacks \"value\"");
	}
	return Ok<>(res);
}

template<>
Result<ColorSetting*> GeodeSetting<ColorSetting>::parse(nlohmann::json const& json) {
	auto res = new ColorSetting;
	res->parseFields(json);
	if (json.contains("value")) {
		auto value = json["value"];
		if (value.is_object()) {
			if (value.contains("default")) {
				auto val = res->parseColor(value["default"]);
				if (val) {
					res->m_default = val.value();
				} else {
					delete res;
					return Err<>(val.error());
				}
			} else {
				delete res;
				return Err<>("Setting has \"value\" but no \"value.default\"");
			}
		} else {
			delete res;
			return Err<>("Setting has \"value\" but it is not an object");
		}
	} else {
		delete res;
		return Err<>("Setting lacks \"value\"");
	}
	return Ok<>(res);
}

template<>
Result<ColorAlphaSetting*> GeodeSetting<ColorAlphaSetting>::parse(nlohmann::json const& json) {
	auto res = new ColorAlphaSetting;
	res->parseFields(json);
	if (json.contains("value")) {
		auto value = json["value"];
		if (value.is_object()) {
			if (value.contains("default")) {
				auto val = res->parseColor(value["default"]);
				if (val) {
					res->m_default = val.value();
				} else {
					delete res;
					return Err<>(val.error());
				}
			} else {
				delete res;
				return Err<>("Setting has \"value\" but no \"value.default\"");
			}
		} else {
			delete res;
			return Err<>("Setting has \"value\" but it is not an object");
		}
	} else {
		delete res;
		return Err<>("Setting lacks \"value\"");
	}
	return Ok<>(res);
}

template<>
Result<PathSetting*> GeodeSetting<PathSetting>::parse(nlohmann::json const& json) {
	auto res = new PathSetting;
	res->parseFields(json);
	if (json.contains("value")) {
		auto value = json["value"];
		if (value.is_object()) {
			if (value.contains("default")) {
				if (value["default"].is_string()) {
					res->m_default = value["default"].get<ghc::filesystem::path>();
				} else {
					delete res;
					return Err<>("Setting has \"value.default\" but it is not a string");
				}
			} else {
				delete res;
				return Err<>("Setting has \"value\" but no \"value.default\"");
			}
		} else {
			delete res;
			return Err<>("Setting has \"value\" but it is not an object");
		}
	} else {
		delete res;
		return Err<>("Setting lacks \"value\"");
	}
	return Ok<>(res);
}

template<>
Result<StringSelectSetting*> GeodeSetting<StringSelectSetting>::parse(nlohmann::json const& json) {
	auto res = new StringSelectSetting;
	res->parseFields(json);
	if (json.contains("value")) {
		auto value = json["value"];
		if (value.is_object()) {
			if (value.contains("options")) {
				if (value["options"].is_array()) {
					for (auto const& opt : value["options"]) {
						if (!opt.is_string()) {
							delete res;
							return Err<>("Option in \"value.options\" is not a string");
						}
						res->m_options.push_back(opt);
					}
				} else {
					delete res;
					return Err<>("Setting has \"value.options\" but it is not an array");
				}
			} else {
				delete res;
				return Err<>("Setting has \"value\" but no \"value.options\"");
			}
			if (value.contains("default")) {
				if (value["default"].is_number_unsigned()) {
					res->m_default = value["default"];
					if (res->m_default > res->m_options.size() - 1) {
						delete res;
						return Err<>("Setting has \"value.default\" but it is higher than the size of \"value.options\"");
					}
				} else {
					delete res;
					return Err<>("Setting has \"value.default\" but it is not an index");
				}
			} else {
				delete res;
				return Err<>("Setting has \"value\" but no \"value.default\"");
			}
		} else {
			delete res;
			return Err<>("Setting has \"value\" but it is not an object");
		}
	} else {
		delete res;
		return Err<>("Setting lacks \"value\"");
	}
	return Ok<>(res);
}

Result<Setting*> Setting::parseFromJSON(nlohmann::json const& json) {
	if (json.is_object()) {
		if (json.contains("type")) {
			if (json["type"].is_string()) {
				switch (hash(string_utils::toLower(json["type"].get<std::string>()).c_str())) {
					case hash("bool"): return GeodeSetting<BoolSetting>::parse(json);
					case hash("int"): return GeodeSetting<IntSetting>::parse(json);
					case hash("float"): return GeodeSetting<FloatSetting>::parse(json);
					case hash("string"): return GeodeSetting<StringSetting>::parse(json);
					case hash("color"): return GeodeSetting<ColorSetting>::parse(json);
					case hash("rgba"): return GeodeSetting<ColorAlphaSetting>::parse(json);
					case hash("file"): return GeodeSetting<PathSetting>::parse(json);
					case hash("string[]"): return GeodeSetting<StringSelectSetting>::parse(json);
				}
				return Err<>("Setting has unrecognized type \"" + json["type"].get<std::string>() + "\"");
			} else {
				return Err<>("Setting has \"type\", but it's not a string");
			}
		} else {
			return Err<>("Setting lacks required field \"type\"");
		}
	} else if (json.is_string()) {
		if (json == "custom") {
			return Ok<>(new CustomSettingPlaceHolder);
		} else {
			return Err<>("Setting is string, but its value is not \"custom\"");
		}
	}
	return Err<>("Setting is neither a string nor an object");
}

void Setting::save(nlohmann::json& json) const {}

void Setting::load(nlohmann::json const& json) {}

#pragma warning(default: 4067)
