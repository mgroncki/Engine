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

#include "toplevelfixture.hpp"
#include <boost/test/unit_test.hpp>
#include <ql/termstructures/yield/piecewiseyieldcurve.hpp>
#include <ql/termstructures/yield/ratehelpers.hpp>
#include <ql/time/calendars/target.hpp>
#include <ql/time/daycounters/actual360.hpp>
#include <qle/pricingengines/depositengine.hpp>

#include <boost/make_shared.hpp>

using namespace QuantLib;
using namespace QuantExt;
using namespace boost::unit_test_framework;

BOOST_FIXTURE_TEST_SUITE(QuantExtTestSuite, qle::test::TopLevelFixture)

BOOST_AUTO_TEST_SUITE(DepositTest)

BOOST_AUTO_TEST_CASE(testRepricing) {

    BOOST_TEST_MESSAGE("Testing Repricing of a Deposit on a depo curve...");

    SavedSettings backup;

    Date refDate = Date(8, Dec, 2016);
    Settings::instance().evaluationDate() = refDate;

    std::vector<QuantLib::ext::shared_ptr<RateHelper> > helper;
    helper.push_back(QuantLib::ext::make_shared<DepositRateHelper>(Handle<Quote>(QuantLib::ext::make_shared<SimpleQuote>(0.02)),
                                                           7 * Months, 2, TARGET(), ModifiedFollowing, false,
                                                           Actual360()));

    Handle<YieldTermStructure> curve(
        QuantLib::ext::make_shared<PiecewiseYieldCurve<Discount, LogLinear> >(refDate, helper, Actual365Fixed()));

    QuantLib::ext::shared_ptr<PricingEngine> engine = QuantLib::ext::make_shared<DepositEngine>(curve);

    Deposit depo(100.0, 0.02, 7 * Months, 2, TARGET(), ModifiedFollowing, false, Actual360(), refDate, true, 0 * Days);
    depo.setPricingEngine(engine);

    Real tol = 1.0E-8;
    BOOST_CHECK_MESSAGE(std::abs(depo.NPV()) <= tol,
                        "Deposit NPV(" << depo.NPV() << ") could not be verified, expected 0.0");

    BOOST_CHECK_MESSAGE(std::abs(depo.fairRate() - 0.02) <= tol,
                        "Deposit fair rate (" << depo.fairRate() << ") could not be verified, expected 0.02");
}

BOOST_AUTO_TEST_SUITE_END()

BOOST_AUTO_TEST_SUITE_END()
