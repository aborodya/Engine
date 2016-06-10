/* -*- mode: c++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */

/*
 Copyright (C) 2016 Quaternion Risk Management Ltd.
*/

/*! \file crossassetanalytics.hpp
    \brief analytics for the cross asset model
*/

#ifndef quantext_crossasset_analytics_hpp
#define quantext_crossasset_analytics_hpp

#include <qle/models/crossassetmodel.hpp>
#include <qle/models/crossassetanalyticsbase.hpp>

using namespace QuantLib;

namespace QuantExt {

    /*! CrossAssetAnalytics 
      
      This namesace provides a number of functions which compute analytical moments
      (expectations and covariances) of cross asset model factors.
      These are used in the exact propagation of cross asset model paths (i.e. without 
      time discretisation error).

      Reference:
      Lichters, Stamm, Gallagher: Modern Derivatives Pricing and Credit Exposure
      Analysis, Palgrave Macmillan, 2015

      See also the documentation in class CrossAssetModel.

      Section 16.1 in the reference above lists the analytical expectations and covariances 
      implemented in this namespace. 
      In the following we consider time intervals \f$(s,t)\f$. We aim at computing conditional 
      expectations of factors at time \f$t\f$ given their state at time \f$s\f$, likewise covariances of 
      factor moves \f$\Delta z\f$ and \f$\Delta x\f$ over time interval \f$(s,t)\f$. 

      Starting with the interest rate processes 
      \f{eqnarray*}{
      dz_i &=& \epsilon_i\,\gamma_i\,dt + \alpha^z_i\,dW^z_i, \qquad \epsilon_i = \left\{ \begin{array}{ll} 0 & i = 0  \\ 1 & i > 0 \end{array}\right.
      \f}
      we get the factor move by integration
      \f{eqnarray*}{
      \Delta z_i &=& -\int_s^t H^z_i\,(\alpha^z_i)^2\,du + \rho^{zz}_{0i} \int_s^t H^z_0\,\alpha^z_0\,\alpha^z_i\,du\\
      && - \epsilon_i  \rho^{zx}_{ii}\int_s^t \sigma_i^x\,\alpha^z_i\,du + \int_s^t \alpha^z_i\,dW^z_i. \\
      \f}
      Thus, conditional expectation and covariances are
      \f{eqnarray*}{
      \mathbb{E}[\Delta z_i] &=& -\int_s^t H^z_i\,(\alpha^z_i)^2\,du + \rho^{zz}_{0i} \int_s^t H^z_0\,\alpha^z_0\,\alpha^z_i\,du
      - \epsilon_i  \rho^{zx}_{ii}\int_s^t \sigma_i^x\,\alpha^z_i\,du \\
      \mathrm{Cov}[\Delta z_a, \Delta z_b] &=& \rho^{zz}_{ab} \int_s^t \alpha^z_a\,\alpha^z_b\,du
      \f}

      Proceeding similarly with the foreign exchange rate processes
      \f[
      dx_i / x_i = \mu^x_i \, dt +\sigma_i^x\,dW^x_i,
      \f]
      we get the following log-moves by integration

      \f{eqnarray*}{
      \Delta \ln x_i &=& \ln \left( \frac{P^n_0(0,s)}{P^n_0(0,t)} \frac{P^n_i(0,t)}{P^n_i(0,s)}\right) - \frac12 \int_s^t (\sigma^x_i)^2\,du + \rho^{zx}_{0i}\int_s^t H^z_0\, \alpha^z_0\, \sigma^x_i \,du\nonumber\\
      &&+\int_s^t \zeta^z_0\,H^z_0\, (H^z_0)^{\prime}\,du-\int_s^t \zeta^z_i\,H^z_i\, (H^z_i)^{\prime}\,du\nonumber\\
      &&+ \int_s^t \left(H^z_0(t)-H^z_0\right)\alpha_0^z\,dW^z_0+ \left(H^z_0(t)-H^z_0(s)\right) z_0(s) \nonumber\\
      &&- \int_s^t \left(H^z_i(t)-H^z_i\right)\alpha_i^z\,dW^z_i  -\left(H^z_i(t)-H^z_i(s)\right)z_i(s) \nonumber\\
      &&- \int_s^t \left(H^z_i(t)-H^z_i\right)\gamma_i\,du + \int_s^t\sigma^x_i dW^x_i \nonumber
      \f}

      Integration by parts yields

      \f{eqnarray*}{
      && \int_s^t \zeta^z_0\,H^z_0\, (H^z_0)^{\prime}\,du-\int_s^t \zeta^z_i\,H^z_i\, (H^z_i)^{\prime}\,du\\
      && = \frac12 \left((H^z_0(t))^2 \zeta^z_0(t) -  (H^z_0(s))^2 \zeta^z_0(s)- \int_s^t (H^z_0)^2 (\alpha^z_0)^2\,du\right)\nonumber\\
      &&\qquad {}- \frac12 \left((H^z_i(t))^2 \zeta^z_i(t) -  (H^z_i(s))^2 \zeta^z_i(s)-\int_s^t (H^z_i)^2 (\alpha^z_i)^2\,du \right)
      \f}

      so that the expectation is
      \f{eqnarray}{
      \mathbb{E}[\Delta \ln x_i] &=& \ln \left( \frac{P^n_0(0,s)}{P^n_0(0,t)} \frac{P^n_i(0,t)}{P^n_i(0,s)}\right) - \frac12 \int_s^t (\sigma^x_i)^2\,du + \rho^{zx}_{0i} \int_s^t H^z_0\, \alpha^z_0\, \sigma^x_i\,du\nonumber\\
      &&+\frac12 \left((H^z_0(t))^2 \zeta^z_0(t) -  (H^z_0(s))^2 \zeta^z_0(s)- \int_s^t (H^z_0)^2 (\alpha^z_0)^2\,du\right)\nonumber\\
      &&-\frac12 \left((H^z_i(t))^2 \zeta^z_i(t) -  (H^z_i(s))^2 \zeta^z_i(s)-\int_s^t (H^z_i)^2 (\alpha^z_i)^2\,du \right)\nonumber\\
      &&+ \left(H^z_0(t)-H^z_0(s)\right) z_0(s) -\left(H^z_i(t)-H^z_i(s)\right)z_i(s)\nonumber\\
      &&  - \int_s^t \left(H^z_i(t)-H^z_i\right)\gamma_i \,du, \label{eq:meanX} 
      \f}

      and IR-FX and FX-FX covariances are
      
      \f{eqnarray}{
      \mathrm{Cov}[\Delta \ln x_a, \Delta \ln x_b] &=&  \int_s^t \left(H^z_0(t)-H^z_0\right)^2 (\alpha_0^z)^2\,du \nonumber\\
      &&- \rho^{zz}_{0b}\int_s^t \left(H^z_0(t)-H^z_0\right)\alpha_0^z \left(H^z_b(t)-H^z_b\right)\alpha_b^z\,du \nonumber\\
      &&+ \rho^{zx}_{0b}\int_s^t \left(H^z_0(t)-H^z_0\right)\alpha_0^z \sigma^x_b\,du \nonumber\\
      && -\rho^{zz}_{0a} \int_s^t \left(H^z_a(t)-H^z_a\right) \alpha_a^z\left(H^z_0(t)-H^z_0\right) \alpha_0^z\,du \nonumber\\
      &&+ \rho^{zz}_{ab}\int_s^t \left(H^z_a(t)-H^z_a\right)\alpha_a^z \left(H^z_b(t)-H^z_b\right)\alpha_b^z\,du \nonumber\\
      &&- \rho^{zx}_{ab}\int_s^t \left(H^z_a(t)-H^z_a\right)\alpha_a^z \sigma^x_b,du\nonumber\\
      &&+ \rho^{zx}_{0a}\int_s^t \left(H^z_0(t)-H^z_0\right)\alpha_0^z\,\sigma^x_a\,du \nonumber\\
      &&- \rho^{zx}_{ba}\int_s^t \left(H^z_b(t)-H^z_b\right)\alpha_b^z\,\sigma^x_a\, du \nonumber\\
      &&+ \rho^{xx}_{ab}\int_s^t\sigma^x_a\,\sigma^x_b \,du \label{eq:covXX}\\
      &&\nonumber\\
      \mathrm{Cov} [\Delta z_a, \Delta \ln x_b]) &=& \rho^{zz}_{0a}\int_s^t \left(H^z_0(t)-H^z_0\right)  \alpha^z_0\,\alpha^z_a\,du \nonumber\\
      &&- \rho^{zz}_{ab}\int_s^t \alpha^z_a \left(H^z_b(t)-H^z_b\right) \alpha^z_b \,du \nonumber\\
      &&+\rho^{zx}_{ab}\int_s^t \alpha^z_a \, \sigma^x_b \,du. \label{eq:covZX}
      \f}

      Based on these expectations of factor moves and log-moves, respectively, we can work out the 
      conditonal expectations of the factor levels at time \f$t\f$. These expectations have state-dependent
      parts (levels at time \f$s\f$) and state-independent parts which we separate in the implementation, 
      see functions ending with "_1" and "_2", respectively.
      Moreover, the implementation splits up the integrals further in order to separate simple and more 
      complex integrations and to allow for tailored efficient numerical integration schemes. 

      In the following we rearrange the expectations and covariances above such that the expressions 
      correspond 1:1 to their implementations below.

      \todo Rearrange integrals to achieve 1:1 correspondence with code
     */

    namespace CrossAssetAnalytics {
    
/*! IR state expectation

  This function evaluates part of the expectation \f$ \mathbb{E}_{t_0}[z_i(t_0+dt)]\f$.

  Using the results above for factor moves \f$\Delta z\f$ over time interval \f$(s,t)\f$, 
  we have

  \f{eqnarray}{
  \mathbb{E}_{t_0}[z_i(t_0+\Delta t)] &=& z_i(t_0) + \mathbb{E}_{t_0}[\Delta z_i], 
  \qquad\mbox{with}\quad \Delta z_i = z_i(t_0+\Delta t) - z_i(t_0) \\
  &=& z_i(t_0) -\int_{t_0}^{t_0+\Delta t} H^z_i\,(\alpha^z_i)^2\,du + \rho^{zz}_{0i} \int_{t_0}^{t_0+\Delta t} H^z_0\,\alpha^z_0\,\alpha^z_i\,du
      - \epsilon_i  \rho^{zx}_{ii}\int_{t_0}^{t_0+\Delta t} \sigma_i^x\,\alpha^z_i\,du
  \f}

  This function covers the latter three integrals, the state-independent part.

*/
Real ir_expectation_1(const CrossAssetModel *model, const Size i, const Time t0,
                      const Real dt);

/*! IR state expecation

  This function evaluates the state-dependent part of the expectation 

  \f{eqnarray}{
  \mathbb{E}_{t_0}[z_i(t_0+\Delta t)] 
  &=& z_i(t_0) -\int_{t_0}^{t_0+\Delta t} H^z_i\,(\alpha^z_i)^2\,du + \rho^{zz}_{0i} \int_{t_0}^{t_0+\Delta t} H^z_0\,\alpha^z_0\,\alpha^z_i\,du
      - \epsilon_i  \rho^{zx}_{ii}\int_{t_0}^{t_0+\Delta t} \sigma_i^x\,\alpha^z_i\,du
  \f}
  
  i.e. simply the first contribution \f$ z_i(t_0) \f$. 

*/
Real ir_expectation_2(const CrossAssetModel *model, const Size i, const Real zi_0);

/*! FX state expectation

  This function evaluates part of the expectation \f$ \mathbb{E}_{t_0}[\ln x_i(t_0+dt)]\f$.

  Using the results above for factor moves \f$\Delta \ln x\f$ over time interval \f$(s,t)\f$, 
  we have

    \f{eqnarray}{
  \mathbb{E}_{t_0}[\ln x_i(t_0+\Delta t)] &=& \ln x_i(t_0) +  \mathbb{E}_{t_0}[\Delta \ln x_i],
  \qquad\mbox{with}\quad \Delta \ln x_i = \ln x_i(t_0+\Delta t) - \ln x_i(t_0) \\
  &=& \ln x_i(t_0) + \left(H^z_0(t)-H^z_0(s)\right) z_0(s) -\left(H^z_i(t)-H^z_i(s)\right)z_i(s)\\
  &&+ \ln \left( \frac{P^n_0(0,s)}{P^n_0(0,t)} \frac{P^n_i(0,t)}{P^n_i(0,s)}\right) \\
  && - \frac12 \int_s^t (\sigma^x_i)^2\,du \\
  &&+\frac12 \left((H^z_0(t))^2 \zeta^z_0(t) -  (H^z_0(s))^2 \zeta^z_0(s)- \int_s^t (H^z_0)^2 (\alpha^z_0)^2\,du\right)\\
  &&-\frac12 \left((H^z_i(t))^2 \zeta^z_i(t) -  (H^z_i(s))^2 \zeta^z_i(s)-\int_s^t (H^z_i)^2 (\alpha^z_i)^2\,du \right)\\
  && + \rho^{zx}_{0i} \int_s^t H^z_0\, \alpha^z_0\, \sigma^x_i\,du \\
  &&  - \int_s^t \left(H^z_i(t)-H^z_i\right)\gamma_i \,du, \qquad\mbox{with}\quad s = t_0, \quad t = t_0+\Delta t
  \f}

  where we rearranged terms so that the state-dependent terms are listed on the first line 
  (containing \f$\ln x_i(t_0), z_i(t_0), z_0(t_0) \f$)
  and all following terms are state-independent (deterministic, just dependent on initial market data and model parameters).

  The last integral above contains \f$\gamma_i\f$ which is (see documentation of the CrossAssetModel)
  \f[
  \gamma_i = -H^z_i\,(\alpha^z_i)^2  + H^z_0\,\alpha^z_0\,\alpha^z_i\,\rho^{zz}_{0i} - \sigma_i^x\,\alpha^z_i\, \rho^{zx}_{ii}.
  \f]
  The very last last integral above is therefore broken up into six terms which we do not list
  here, but which show up in this function's imlementation.

  This function covers the latter state-independent part of the FX expectation.
*/
Real fx_expectation_1(const CrossAssetModel *model, const Size i, const Time t0,
                      const Real dt);

/*! FX state expectation.
  
  This function covers the state-dependent part of the FX expectation. 
*/
Real fx_expectation_2(const CrossAssetModel *model, const Size i, const Time t0,
                      const Real xi_0, const Real zi_0, const Real z0_0,
                      const Real dt);

/*! ir-ir covariance */
Real ir_ir_covariance(const CrossAssetModel *model, const Size i, const Size j,
                      const Time t0, const Time dt);

/*! ir-fx covariance */
Real ir_fx_covariance(const CrossAssetModel *model, const Size i, const Size j,
                      const Time t0, const Time dt);

/*! fx-fx covariance */
Real fx_fx_covariance(const CrossAssetModel *model, const Size i, const Size j,
                      const Time t0, const Time dt);

/*! IR H component */
struct Hz {
    Hz(const Size i) : i_(i) {}
    Real eval(const CrossAssetModel *x, const Real t) const {
        return x->irlgm1f(i_)->H(t);
    }
    const Size i_;
};

/*! IR alpha component */
struct az {
    az(const Size i) : i_(i) {}
    Real eval(const CrossAssetModel *x, const Real t) const {
        return x->irlgm1f(i_)->alpha(t);
    }
    const Size i_;
};

/*! IR zeta component */
struct zeta {
    zeta(const Size i) : i_(i) {}
    Real eval(const CrossAssetModel *x, const Real t) const {
        return x->irlgm1f(i_)->zeta(t);
    }
    const Size i_;
};

/*! FX zeta component */
struct sx {
    sx(const Size i) : i_(i) {}
    Real eval(const CrossAssetModel *x, const Real t) const {
        return x->fxbs(i_)->sigma(t);
    }
    const Size i_;
};

/*! IR-IR correlation component */
struct rzz {
    rzz(const Size i, const Size j) : i_(i), j_(j) {}
    Real eval(const CrossAssetModel *x, const Real) const {
        return x->ir_ir_correlation(i_, j_);
    }
    const Size i_, j_;
};

/*! IR-FX correlation component */
struct rzx {
    rzx(const Size i, const Size j) : i_(i), j_(j) {}
    Real eval(const CrossAssetModel *x, const Real) const {
        return x->ir_fx_correlation(i_, j_);
    }
    const Size i_, j_;
};

/*! FX-FX correlation component */
struct rxx {
    rxx(const Size i, const Size j) : i_(i), j_(j) {}
    Real eval(const CrossAssetModel *x, const Real) const {
        return x->fx_fx_correlation(i_, j_);
    }
    const Size i_, j_;
};


} // namespace CrossAssetAnalytics

} // namesapce QuantExt

#endif
