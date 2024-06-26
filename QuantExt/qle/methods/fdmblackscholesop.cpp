/*
 Copyright (C) 2020 Quaternion Risk Management Ltd
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

#include <qle/methods/fdmblackscholesop.hpp>

#include <ql/instruments/payoffs.hpp>
#include <ql/math/functional.hpp>
#include <ql/math/comparison.hpp>
#include <ql/methods/finitedifferences/meshers/fdmmesher.hpp>
#include <ql/methods/finitedifferences/operators/fdmlinearoplayout.hpp>
#include <ql/methods/finitedifferences/operators/secondderivativeop.hpp>

namespace QuantExt {

FdmBlackScholesOp::FdmBlackScholesOp(const ext::shared_ptr<FdmMesher>& mesher,
                                     const ext::shared_ptr<GeneralizedBlackScholesProcess>& bsProcess, Real strike,
                                     bool localVol, Real illegalLocalVolOverwrite, Size direction,
                                     const ext::shared_ptr<FdmQuantoHelper>& quantoHelper, const bool discounting,
                                     const bool ensureNonNegativeForwardVariance)
    : mesher_(mesher), rTS_(bsProcess->riskFreeRate().currentLink()), qTS_(bsProcess->dividendYield().currentLink()),
      volTS_(bsProcess->blackVolatility().currentLink()),
      localVol_((localVol) ? bsProcess->localVolatility().currentLink() : ext::shared_ptr<LocalVolTermStructure>()),
      x_((localVol) ? Array(Exp(mesher->locations(direction))) : Array()), dxMap_(FirstDerivativeOp(direction, mesher)),
      dxxMap_(SecondDerivativeOp(direction, mesher)), mapT_(direction, mesher), strike_(strike),
      illegalLocalVolOverwrite_(illegalLocalVolOverwrite), direction_(direction), quantoHelper_(quantoHelper),
      initialValue_(bsProcess->x0()), discounting_(discounting),
      ensureNonNegativeForwardVariance_(ensureNonNegativeForwardVariance) {}

void FdmBlackScholesOp::setTime(Time t1, Time t2) {
    const Rate r = rTS_->forwardRate(t1, t2, Continuous).rate();
    const Rate q = qTS_->forwardRate(t1, t2, Continuous).rate();
    Rate discountRate = discounting_ ? r : 0.0;

    if (localVol_) {
        const ext::shared_ptr<FdmLinearOpLayout> layout = mesher_->layout();
        const FdmLinearOpIterator endIter = layout->end();

        Array v(layout->size());
        for (FdmLinearOpIterator iter = layout->begin(); iter != endIter; ++iter) {
            const Size i = iter.index();

            if (illegalLocalVolOverwrite_ < 0.0) {
                v[i] = squared(localVol_->localVol(0.5 * (t1 + t2), x_[i], true));
            } else {
                try {
                    v[i] = squared(localVol_->localVol(0.5 * (t1 + t2), x_[i], true));
                } catch (Error&) {
                    v[i] = squared(illegalLocalVolOverwrite_);
                }
            }
        }

        if (quantoHelper_) {
            mapT_.axpyb(r - q - 0.5 * v - quantoHelper_->quantoAdjustment(Sqrt(v), t1, t2), dxMap_,
                        dxxMap_.mult(0.5 * v), Array(1, -discountRate));
        } else {
            mapT_.axpyb(r - q - 0.5 * v, dxMap_, dxxMap_.mult(0.5 * v), Array(1, -discountRate));
        }
    } else {
        Real k1, k2;
        if (strike_ == Null<Real>()) {
            k1 = initialValue_ * qTS_->discount(t1) / rTS_->discount(t1);
            k2 = initialValue_ * qTS_->discount(t2) / rTS_->discount(t2);
        } else {
            k1 = k2 = strike_;
        }
        Real v = ((close_enough(t2, 0.0) ? 0.0 : volTS_->blackVariance(t2, k2)) -
                  (close_enough(t1, 0.0) ? 0.0 : volTS_->blackVariance(t1, k1))) /
                 (t2 - t1);
        if (ensureNonNegativeForwardVariance_)
            v = std::max(v, 0.0);

        if (quantoHelper_) {
            mapT_.axpyb(Array(1, r - q - 0.5 * v) - quantoHelper_->quantoAdjustment(Array(1, std::sqrt(v)), t1, t2),
                        dxMap_, dxxMap_.mult(0.5 * Array(mesher_->layout()->size(), v)), Array(1, -discountRate));
        } else {
            mapT_.axpyb(Array(1, r - q - 0.5 * v), dxMap_, dxxMap_.mult(0.5 * Array(mesher_->layout()->size(), v)),
                        Array(1, -discountRate));
        }
    }
}

Size FdmBlackScholesOp::size() const { return 1u; }

Array FdmBlackScholesOp::apply(const Array& u) const { return mapT_.apply(u); }

Array FdmBlackScholesOp::apply_direction(Size direction, const Array& r) const {
    if (direction == direction_)
        return mapT_.apply(r);
    else {
        Array retVal(r.size(), 0.0);
        return retVal;
    }
}

Array FdmBlackScholesOp::apply_mixed(const Array& r) const {
    Array retVal(r.size(), 0.0);
    return retVal;
}

Array FdmBlackScholesOp::solve_splitting(Size direction, const Array& r, Real dt) const {
    if (direction == direction_)
        return mapT_.solve_splitting(r, dt, 1.0);
    else {
        Array retVal(r);
        return retVal;
    }
}

Array FdmBlackScholesOp::preconditioner(const Array& r, Real dt) const {
    return solve_splitting(direction_, r, dt);
}

#if !defined(QL_NO_UBLAS_SUPPORT)
    std::vector<QuantLib::SparseMatrix> FdmBlackScholesOp::toMatrixDecomp() const {
        std::vector<QuantLib::SparseMatrix> retVal(1, mapT_.toMatrix());
    return retVal;
}
#endif
} // namespace QuantExt
