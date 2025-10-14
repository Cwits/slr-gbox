// SPDX-FileCopyrightText: 2025 Cwits
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

//GENERATE_INCLUDES

#include "Status.h"
#include "defines.h"
#include <cstdint>

namespace slr {

namespace FlatEvents {

struct FlatControl {
	enum class Type { //Error = 0, 
		//GENERATE_CTL_TYPES
	};
	Type type;
	ID commandId;
	union {
        //GENERATE_CTL_UNIONS
	};
};

struct FlatResponse {
	enum class Type { //Error = 0, 
		//GENERATE_RESP_TYPES
	}; 
	Type type;
	Status status;
	ID commandId;
	union {
		//GENERATE_RESP_UNIONS
	};
};

} //namespace FlatEvents

} //namespace slr
