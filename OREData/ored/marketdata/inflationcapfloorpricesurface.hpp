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

/*! \file ored/marketdata/inflationcapfloorpricesurface.hpp
    \brief inflation cap floor price surface class
    \ingroup curves
*/

#pragma once

#include <ql/termstructures/inflationtermstructure.hpp>
#include <ql/termstructures/volatility/inflation/cpivolatilitystructure.hpp>

#include <ored/configuration/conventions.hpp>
#include <ored/configuration/curveconfigurations.hpp>
#include <ored/configuration/inflationcapfloorpricesurfaceconfig.hpp>
#include <ored/marketdata/curvespec.hpp>
#include <ored/marketdata/inflationcurve.hpp>
#include <ored/marketdata/loader.hpp>
#include <ored/marketdata/yieldcurve.hpp>

namespace ore {
namespace data {
using QuantLib::Date;
using QuantLib::InflationTermStructure;
using QuantLib::CPICapFloorTermPriceSurface;
using QuantLib::CPIVolatilitySurface;
using ore::data::CurveConfigurations;
using ore::data::Conventions;

//! Wrapper class for building inflation curves
/*!
  \ingroup curves
*/
class InflationCapFloorPriceSurface {
public:
    InflationCapFloorPriceSurface() {}
    InflationCapFloorPriceSurface(Date asof, InflationCapFloorPriceSurfaceSpec spec, const Loader& loader,
                                  const CurveConfigurations& curveConfigs,
                                  map<string, boost::shared_ptr<YieldCurve>>& yieldCurves,
                                  map<string, boost::shared_ptr<InflationCurve>>& inflationCurves);

    const InflationCapFloorPriceSurfaceSpec& spec() const { return spec_; }

    const boost::shared_ptr<InflationTermStructure> inflationCapFloorPriceSurface() const { return surface_; }
    const boost::shared_ptr<QuantExt::YoYOptionletVolatilitySurface> yoyInflationCapFloorVolSurface() const {
        return yoyVolSurface_;
    }
    bool useMarketYoyCurve() const { return useMarketYoyCurve_; }
    const boost::shared_ptr<YoYInflationTermStructure> yoyInflationAtmCurve() const { return yoyTs_; }
    const boost::shared_ptr<QuantLib::CPIVolatilitySurface> cpiInflationCapFloorVolSurface() const {
        return cpiCapFloorVolSurface_;
    }

private:
    InflationCapFloorPriceSurfaceSpec spec_;
    boost::shared_ptr<InflationTermStructure> surface_;
    boost::shared_ptr<QuantExt::YoYOptionletVolatilitySurface> yoyVolSurface_;
    bool useMarketYoyCurve_;
    boost::shared_ptr<YoYInflationTermStructure> yoyTs_;
    boost::shared_ptr<QuantLib::CPIVolatilitySurface> cpiCapFloorVolSurface_;
};
} // namespace data
} // namespace ore
