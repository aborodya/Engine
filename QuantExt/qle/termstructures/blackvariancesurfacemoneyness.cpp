/*
 Copyright (C) 2017 Quaternion Risk Management Ltd
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

#include <ql/math/interpolations/bilinearinterpolation.hpp>
#include <ql/quotes/simplequote.hpp>
#include <qle/termstructures/blackvariancesurfacemoneyness.hpp>
#include <boost/make_shared.hpp>

namespace QuantExt {

BlackVarianceSurfaceMoneyness::BlackVarianceSurfaceMoneyness(
    const Calendar& cal, const Handle<Quote>& spot, const std::vector<Time>& times, const std::vector<Real>& moneyness,
    const std::vector<std::vector<Handle<Quote> > >& blackVolMatrix, const DayCounter& dayCounter, bool stickyStrike, 
    bool atmf, const Handle<YieldTermStructure>& forTS, const Handle<YieldTermStructure>& domTS)
    : BlackVarianceTermStructure(0, cal), spot_(spot), dayCounter_(dayCounter), moneyness_(moneyness),
      quotes_(blackVolMatrix), atmf_(atmf), forTS_(forTS), domTS_(domTS) {

    QL_REQUIRE(times.size() == blackVolMatrix.front().size(), "mismatch between times vector and vol matrix colums");
    QL_REQUIRE(moneyness_.size() == blackVolMatrix.size(),
               "mismatch between moneyness vector and vol matrix rows");

    QL_REQUIRE(times[0] >= 0, "cannot have times[0] < 0");

    if(atmf_) {
        QL_REQUIRE(!forTS_.empty(), "foreign discount curve required for atmf surface");
        QL_REQUIRE(!domTS_.empty(), "foreign discount curve required for atmf surface");
        registerWith(forTS_);
        registerWith(domTS_);
    }

    if (stickyStrike) {
        // we don't want to know if the spot has changed - we take a copy here
        spot_ = Handle<Quote>(boost::make_shared<SimpleQuote>(spot->value()));
    } else {
        registerWith(spot_);
    }

    Size j, i;
    // internally times_ is one bigger than the input "times"
    times_ = std::vector<Time>(times.size() + 1);
    times_[0] = 0.0;
    variances_ = Matrix(moneyness_.size(), times.size() + 1);
    for (i = 0; i < moneyness_.size(); i++) {
        variances_[i][0] = 0.0;
    }
    for (j = 1; j <= times.size(); j++) {
        times_[j] = times[j - 1];
        QL_REQUIRE(times_[j] > times_[j - 1], "dates must be sorted unique!");
        for (i = 0; i < moneyness_.size(); i++) {
            variances_[i][j] = 0.0;
            registerWith(blackVolMatrix[i][j - 1]);
        }
    }

    varianceSurface_ =
        Bilinear().interpolate(times_.begin(), times_.end(), moneyness_.begin(), moneyness_.end(), variances_);
    notifyObservers();

}

void BlackVarianceSurfaceMoneyness::update() {
    TermStructure::update();
    LazyObject::update();
}

void BlackVarianceSurfaceMoneyness::performCalculations() const {
    for (Size j = 1; j < variances_.columns(); j++) {
        for (Size i = 0; i < variances_.rows(); i++) {
            Real vol = quotes_[i][j - 1]->value();
            variances_[i][j] = times_[j] * vol * vol;
        }
    }
    varianceSurface_.update();
}

Real BlackVarianceSurfaceMoneyness::blackVarianceImpl(Time t, Real strike) const {

    calculate();

    if (t == 0.0)
        return 0.0;

    Real moneyness;
    if (strike == Null<Real>() || strike == 0)
        moneyness = 1.0;
    else if ( !atmf_ )
        moneyness = strike / spot_->value();
    else {
        Real fwd = spot_->value()*forTS_->discount(t)/domTS_->discount(t);
        moneyness = strike / fwd;
    }   
    return blackVarianceMoneyness(t, moneyness);
}

Real BlackVarianceSurfaceMoneyness::blackVarianceMoneyness(Time t, Real m) const {
    if (t <= times_.back())
        return varianceSurface_(t, m, true);
    else
        return varianceSurface_(times_.back(), m, true) * t / times_.back();
}

} // namespace QuantExt
