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

#include <ored/portfolio/builders/fxforward.hpp>

#include <qle/pricingengines/mccamfxforwardengine.hpp>

namespace ore {
namespace data {

using namespace QuantLib;
using namespace QuantExt;

boost::shared_ptr<PricingEngine> CamAmcFxForwardEngineBuilder::engineImpl(const Currency& forCcy,
                                                                          const Currency& domCcy) {

    QL_REQUIRE(domCcy != forCcy, "CamAmcFxForwardEngineBuilder: domCcy = forCcy = " << domCcy.code());

    std::vector<Size> externalModelIndices;
    std::vector<Handle<YieldTermStructure>> discountCurves;
    std::vector<Size> cIdx;
    std::vector<boost::shared_ptr<IrModel>> lgm;
    std::vector<boost::shared_ptr<FxBsParametrization>> fx;

    // add the IR and FX components in the order they appear in the CAM; this way
    // we can sort the external model indices and be sure that they match up with
    // the indices 0,1,2,3,... of the projected model we build here
    // keep the base ccy in every case
    for (Size i = 0; i < cam_->components(CrossAssetModel::AssetType::IR); ++i) {
        if (i == 0 || cam_->irlgm1f(i)->currency() == domCcy || cam_->irlgm1f(i)->currency() == forCcy) {
            lgm.push_back(cam_->lgm(i));
            externalModelIndices.push_back(cam_->pIdx(CrossAssetModel::AssetType::IR, i));
            cIdx.push_back(cam_->cIdx(CrossAssetModel::AssetType::IR, i));
            if (i > 0) {
                fx.push_back(cam_->fxbs(i - 1));
                externalModelIndices.push_back(cam_->pIdx(CrossAssetModel::AssetType::FX, i - 1));
                cIdx.push_back(cam_->cIdx(CrossAssetModel::AssetType::FX, i - 1));
            }
        }
    }

    std::sort(externalModelIndices.begin(), externalModelIndices.end());
    std::sort(cIdx.begin(), cIdx.end());

    // build correlation matrix
    Matrix corr(cIdx.size(), cIdx.size(), 1.0);
    for (Size i = 0; i < cIdx.size(); ++i) {
        for (Size j = 0; j < i; ++j) {
            corr(i, j) = corr(j, i) = cam_->correlation()(cIdx[i], cIdx[j]);
        }
    }
    Handle<CrossAssetModel> model(boost::make_shared<CrossAssetModel>(lgm, fx, corr));
    // we assume that the model has the pricing discount curves attached already, so
    // we leave the discountCurves vector empty here

    // build the pricing engine

    // NPV should be in domCcy, consistent with the npv currency of an ORE FX Forward Trade
    auto engine = boost::make_shared<McCamFxForwardEngine>(
        model, domCcy, forCcy, domCcy, parseSequenceType(engineParameter("Training.Sequence")),
        parseSequenceType(engineParameter("Pricing.Sequence")), parseInteger(engineParameter("Training.Samples")),
        parseInteger(engineParameter("Pricing.Samples")), parseInteger(engineParameter("Training.Seed")),
        parseInteger(engineParameter("Pricing.Seed")), parseInteger(engineParameter("Training.BasisFunctionOrder")),
        parsePolynomType(engineParameter("Training.BasisFunction")),
        parseSobolBrownianGeneratorOrdering(engineParameter("BrownianBridgeOrdering")),
        parseSobolRsgDirectionIntegers(engineParameter("SobolDirectionIntegers")), discountCurves, simulationDates_,
        externalModelIndices, parseBool(engineParameter("MinObsDate")));

    return engine;
}

} // namespace data
} // namespace ore
