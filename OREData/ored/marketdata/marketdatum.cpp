/*
 Copyright (C) 2016 Quaternion Risk Management Ltd
 All rights reserved.

 This file is part of ORE, a free-software/open-source library
 for transparent pricing and risk analysis - http://opensourcerisk.org

 ORE is free software: you can redistribute it and/or modify it
 under the terms of the Modified BSD License.  You should have received a
 copy of the license along with this program.
 The license is also available online at <http://opensourcerisk.org>

 This program is distributed on the basis that it will form a useful
 contribution to risk analytics and model standardisation, but WITHOUT
 ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 FITNESS FOR A PARTICULAR PURPOSE. See the license for more details.
*/

#include <boost/lexical_cast.hpp>
#include <ored/marketdata/marketdatum.hpp>

using boost::lexical_cast;
using boost::bad_lexical_cast;

namespace ore {
namespace data {

Natural MMFutureQuote::expiryYear() const {
    QL_REQUIRE(expiry_.length() == 7, "The expiry string must be of "
                                      "the form YYYY-MM");
    string strExpiryYear = expiry_.substr(0, 4);
    Natural expiryYear;
    try {
        expiryYear = lexical_cast<Natural>(strExpiryYear);
    } catch (const bad_lexical_cast&) {
        QL_FAIL("Could not convert year string, " << strExpiryYear << ", to number.");
    }
    return expiryYear;
}

Month MMFutureQuote::expiryMonth() const {
    QL_REQUIRE(expiry_.length() == 7, "The expiry string must be of "
                                      "the form YYYY-MM");
    string strExpiryMonth = expiry_.substr(5);
    Natural expiryMonth;
    try {
        expiryMonth = lexical_cast<Natural>(strExpiryMonth);
    } catch (const bad_lexical_cast&) {
        QL_FAIL("Could not convert month string, " << strExpiryMonth << ", to number.");
    }
    return static_cast<Month>(expiryMonth);
}
} // namespace data
} // namespace ore
