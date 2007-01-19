/**************************************************************************/
/*    Copyright (C) 2006 Romain Michalec                                  */
/*                                                                        */
/*    This library is free software; you can redistribute it and/or       */
/*    modify it under the terms of the GNU Lesser General Public          */
/*    License as published by the Free Software Foundation; either        */
/*    version 2.1 of the License, or (at your option) any later version.  */
/*                                                                        */
/*    This library is distributed in the hope that it will be useful,     */
/*    but WITHOUT ANY WARRANTY; without even the implied warranty of      */
/*    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU   */
/*    Lesser General Public License for more details.                     */
/*                                                                        */
/*    You should have received a copy of the GNU Lesser General Public    */
/*    License along with this library; if not, write to the Free Software   */
/*    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston,          */
/*    MA  02110-1301, USA                                                 */
/*                                                                        */
/**************************************************************************/

#include "wmw-test.h"
#include "shared.h"
#include <cmath>
#include <gsl/gsl_cdf.h>


/* WILCOXON-MANN-WHITNEY RANKS TEST
 *
 * This long explaination provides you with everything you must know to
 * understand totally the Wilcoxon-Mann-Whitney test.
 *
 * We consider a problem in which random samples are taken from two
 * populations (sample1 from population 1, sample2 from population 2),
 * and we must determine, on the basis of the observed values in the
 * samples, whether they come from the same distribution or not.
 *
 * The hypotheses to be tested are as follow:
 *
 *     H0: sample1 and sample2 are taken from distributions having
 *         the same distribution function
 *     H1: sample1 and sample2 are taken from distributions having
 *         different distribution functions
 *
 * To determine which hypothesis is the most likely, we must shape
 * a test statistic. To do so, we will use a kind of ranksum for each
 * sample as a means of measuring the differences between them (as
 * measure of difference is synonymous of test statistic). Hence the
 * name "ranks test".
 *
 * We begin by arranging the n_sum = n1 + n2 observations in the two
 * samples in a single sequence, from the smallest to the largest
 * value. To make it simpler, we will assume that all n1+n2 values
 * are different, so that we can obtain a total ordering of the values.
 * Each observation in this ordering is then assigned a rank from 1
 * (for the smallest value) to n_sum (for the largest value): these
 * ranks correspond to the positions of the values in the ordering.
 *
 * (Note: what happens if some values in the n1+n2 values are the same?
 * Basically, we make a kind of mean ranking. Suppose that the following
 * (sorted) values are observed:
 *     sample 1: 1, 2, 4, 5, 5, 5, 7, ...      sample 2: 5, 6, ...
 * When rearranging these value into the sorted sequence, we get four "5"
 * at positions 4, 5, 6 and 7. The mean of these positions being 5.5, we
 * use 5.5 as common rank for all "5", and add 3*5.5 to the ranksum of
 * sample 1 and 1*5.5 to the ranksum of sample 2.)
 *
 * The test is based on the idea that if the null hypothesis is true,
 * then the two samples are actually drawn from the same distribution,
 * and the observations of each sample will tend to be dispersed
 * equally throughout the n1+n2 observations, rather than be
 * concentrated among the smaller values, among the larger values or
 * among the median values. In fact, if the null hypothesis is true,
 * the ranks of the n1 observations of sample1 will be the same as if
 * they were a random sample of n1 ranks drawn at random from the
 * n_sum ranks; same thing, of course, for the n2 observations in
 * sample2.
 *
 * Let X_1, ..., X_n denote the first sample and Y_1, ..., Y_m
 * denote the second sample; let Xr_1, ..., Xr_n denote the ranks of
 * the first sample and S = Xr_1 + ... + Xr_n denote the ranksum of
 * this first sample. We also let mu and sigma denote the mean and
 * standard deviation of sample Xr_1, ..., Xr_n; S/n is the empirical
 * value of mu.
 *
 * If we use Laplace's central limit theorem on sample Xr_1, ..., Xr_n,
 * we get that when n --> +infty, the distribution function of the
 * random variable sqrt(n)*(S/n - mu) is a normal distribution with
 * mean 0 and variance sigma^2. After some rewriting work, this result
 * states that when sample size n is large, the distribution of S is
 * approximately a normal distribution; let's E[S] and Var[S] denote its
 * mean and variance. This result is of crucial importance, because a
 * normal distribution may be computerised, while the unknown distribution
 * functions of the samples could absolutely not be handled easily!
 *
 * We can easily compute the value of E[S], the expectation value of the
 * ranksum of sample X_1, ..., X_n (this value is denoted exp_rs1 in the
 * following source code, and the experimental value S is denoted rs1).
 * Indeed, when n and m are large and the null hypothesis is right, the
 * ranks Xr_1, ..., Xr_n are the same as is they were a random sample of
 * n ranks drawn at random from the n+m ranks. Therefore, the mean of the
 * sum of the ranks E[S] is equal to the sum of the average of the ranks:
 *
 *     E[S] = n * (1 + (n+m))/2
 *
 * Summary: when n and m are large and the null hypothesis is true, the
 * value of S is likely to be close to its "H0-true-assumed" value,
 * n*(1+n+m)/2. If the value of S is found to deviate from this expectation
 * value, then it is less likely that H0 is true.
 *
 * Hence the idea of using S as a test statistic, and reject H0 as false
 * when its value deviates too far from n*(1+n+m)/2:
 *
 *     Reject H0 when | S - n*(1+n+m)/2 | >= c
 *
 * where c is a constant, the "critical value", chosen such as to make
 * the test have the desired significance level alpha.
 *
 * Namely, c is the (1-alpha/2)-quantile of the distribution function
 * of the random variable S - n*(1+n+m)/2, i.e. the (1-alpha/2)-quantile
 * of the normal distribution with mean 0 and variance Var[S].
 *
 * Explanation: if we know alpha (traditionaly, alpha = 0.05) and are
 * looking for the critical value c, we write that:
 *
 *     alpha = Pr ( H_0 rejected | H_0 true )
 *           = Pr ( | S - n*(1+n+m)/2 | >= c  |  H_0 true )
 *
 * We said previously that when H_0 was true (a fact that is assumed here:
 * | H_0 true), the distribution function of S was a normal distribution
 * with mean E[S] = n*(1+n+m)/2. With the help of a little drawing, we
 * interpret the last equation:
 *
 *     alpha is the sum of the areas under the curve of the normal
 *     distribution with mean n*(1+n+m)/2 that are located further
 *     away from the mean n*(1+n+m)/2 than c, i.e. the areas under
 *     the curve and above the intervals ]-infty, n*(1+n+m)/2 - c]
 *     and [n*(1+n+m)/2 + c, infty[.
 *
 * As these two areas are symetrical, each equals alpha/2, and we get:
 *
 *     1 - alpha/2 is the area under the curve from -infty
 *     to n*(1+n+m)/2 + c
 *
 * i.e.:
 *
 *     1 - alpha/2 = Pr (S < n*(1+n+m)/2 + c)
 *                 = Pr (S - n*(1+n+m)/2 < c)
 *
 * The last equation reads: c is the (1 - alpha/2)-quantile of the
 * distribution function of the random variable S - n*(1+n+m)/2, i.e.
 * of the normal distribution with mean 0 and variance Var[S].
 * "Quod erat demonstrantum", and our test procedure now reads:
 *
 *     Reject H0 to significance level alpha when | S - n*(1+n+m)/2 | >= c
 *
 * But as we know, this kind of testing is quite crude, because it only raises
 * a binary answer. We should be more interested in the p-values of the
 * observations, i.e. in the smallest significance levels to which we would
 * reject H0, with respect to the observations (the data; there is one p-value 
 * per observation, i.e. per value of the test statistic, per time window).
 * Our test would then say:
 *
 *     Considering the data we have, reject H0 to any significance level
 *     greater than the p-value of the data
 *
 * which tells us more information.
 *
 * Depending on whether the p-value is low or high, we will have an
 * idea of how strong or weak is the evidence against H0 provided by
 * the data. For example, we will be much more comfortable with
 * rejecting H0 if the p-value is 0.0045 than if it is 0.0493. 
 *
 * The p-value of a test when the observed data yields a value s for the
 * test statistic S is nothing more than the significance level p = alpha
 * corresponding to the critical value c = |s - n*(1+n+m)/2|, gap between
 * the observed value and the expectation value (in the case of a test
 * "Reject H0 when |S-thing|>c"). Therefore:
 *
 *     * if s >= n*(1+n+m)/2:
 *
 *       p = Pr ( H_0 rejected | H_0 true )
 *         = Pr ( | S - n*(1+n+m)/2 | >= s - n*(1+n+m)/2 |  H_0 true )
 *       => (drawing) =>
 *       Pr ( S <=  s | H_0 true) = 1 - p/2
 *
 *       i.e. s is the (1 - p/2)-quantile of the normal distribution
 *       with mean n*(1+n+m)/2 and variance Var[S]:
 *
 *       1 - p/2 = Pr (S <= s) = S_cdf(s)
 *
 *       p = 2*(1-S_cdf(s))
 *
 *     * if s <= n*(1+n+m)/2:
 *
 *       p = Pr ( H_0 rejected | H_0 true )
 *         = Pr ( | S - n*(1+n+m)/2 | >= n*(1+n+m)/2 - s |  H_0 true )
 *       => (drawing) =>
 *       Pr ( S <=  s | H_0 true) = p/2
 *
 *       i.e. s is the p/2-quantile of the normal distribution
 *       with mean n*(1+n+m)/2 and variance Var[S]:
 *
 *       p/2 = Pr (S <= s) = S_cdf(s)
 *
 *       p = 2*S_cdf(s)
 *
 * S_cdf denotes the cumulative distribution function of random variable S,
 * i.e. an approximation of the c.d.f. of a normal distribution with
 * mean n*(1+n+m)/2 and variance Var[S].
 *
 * This value may be computed thanks to the function
 * gsl_cdf_gaussian_P(d, Sigma) from the GNU Scientific Library,
 * which computes the c.d.f. in d of a random variable following a gaussian
 * distribution with mean 0 and standard deviation Sigma, i.e. also the c.d.f.
 * in Mu+d of a random variable following a gaussian distribution with mean
 * Mu and standard deviation Sigma. Here, Mu = n*(1+n+m)/2.
 *
 * The GSL also provides the function:
 *
 *     gsl_cdf_gaussian_Q(d,Sigma) = 1 - gsl_cdf_gaussian_P(d,Sigma)
 *
 * Last thing to know: when we look at the form of the test, it appears that
 * Wilcoxon-Mann-Whitney test is a two-sided test:
 *
 *     H0: |S - n*(1+n+m)/2| <  c
 *     H1: |S - n*(1+n+m)/2| >= c
 *
 * However, contrary to Pearson's Chi-square test, it is possible and easy
 * to adapt this test as a one-sided test that makes sense (c > 0):
 *
 *     H0: S - n*(1+n+m)/2 <  c
 *     H1: S - n*(1+n+m)/2 >= c
 *
 * Explanation / interpretation: suppose S is the ranksum of the observations
 * collected in the "new" sample -- recent network traffic; it is usually
 * "sample2", Y_1, ..., Y_n; "sample1", X_1, ..., X_m, being the past
 * network traffic.
 * In the first form of this test, we will consider as network anomalies
 * all the variations in the network traffic that make the "new" sample
 * different from the "old" one, and S too far away from n*(1+n+m)/2.
 * In the second form, however, we will consider as anomalies only the
 * increases in network traffic, as the null hypothesis, S - n*(1+n+m)/2 < c,
 * means that the ranks of the "new" sample are concentrated among the median
 * and smaller values of all the n+m observations. These anomalies are more
 * likely to be attacks than the others.
 *
 * For the one-sided version, "Reject H0 if S >= c + n*(1+n+m)/2", supposing
 * s is the value of the test statistic, the p-value is the significance
 * level that corresponds to the critical value c + n*(1+n+m)/2 = s:
 *
 *     p = Pr ( H_0 rejected | H_0 true )
 *       = Pr ( S - n*(1+n+m)/2 >= s - n*(1+n+m)/2 |  H_0 true )
 *       = Pr ( S >= s | H_0 true )
 *     => (drawing) =>
 *     Pr ( S <=  s | H_0 true) = 1 - p
 *
 *     i.e. s is the (1 - p)-quantile of the normal distribution
 *     with mean n*(1+n+m)/2 and variance Var[S]:
 *
 *     1 - p = Pr (S <= s) = S_cdf(s)
 *
 *     p = 1 - S_cdf(s)
 *
 * Thanks for the reading; now you know everything useful to
 * understand entirely the following function.
 */


/* This function returns an approximated p-value
 * of the Wilcoxon-Mann-Whitney test.
 *
 * The hypothesis to be tested are:
 *
 *     H0: the samples are drawn from populations having same distribution
 *     H1: they come from populations with different distributions
 *
 * The test statistic is
 *
 *     S = sum of the ranks of one sample among the sorted n1+n2 ranks
 *
 * (notations of the text above); here we compute the sum of the ranks of the
 * _second_ sample (to have those of the first one, uncomment the appropriate
 * lines at the end of the function).
 *
 * The p-value when the observed value for S is s is computed through:
 *
 * Two-sided:
 * p = 2 * (1 - Pr(S<=s)) = 2 * (1 - S_cdf(s))
 *   = 2 * (1 - gsl_cdf_gaussian_P(s - n2*(1+n1+n2)/2) )
 *   if s >= n2*(1+n1+n2)/2
 * p = 2 * Pr(S<=s) = 2 * S_cdf(s)
 *   = 2 * gsl_cdf_gaussian_P(s - n2*(1+n1+n2)/2)
 *   if s <= n2*(1+n1+n2)/2
 *
 * One-sided:
 * p = 1 - Pr(S<=s) = 1 - S_cdf(s)
 *   = 1 - gsl_cdf_gaussian_P(s - n2*(1+n1+n2)/2)
 *
 * (Proofs are in the above text.)
 *
 * We reject H0 to significance level alpha if p-value < alpha,
 * i.e. considering the data we have, we reject H0 to any significance
 * level alpha > p-value.
 *
 * This test has large power only if one of the two distributions
 * is stochastically larger than the other. Both sample sizes have
 * to be >= 8, for excellent approximation n1+n2 > 60.
 */
double wmw_test( std::list<int64_t> sample1, std::list<int64_t> sample2,
		 bool twosided, std::ofstream & outfile ) {

  unsigned int n1, n2, n_sum;
    // sample sizes

  float rs1, rs2;
    // sums of ranks of the two samples
  float exp_rs1, exp_rs2;
    // expectation values of the sums of the ranks of the two samples

  int64_t tie;
    // tied value
  unsigned int to1, to2, to;
    // numbers of tie occurences of a particular tied value
  unsigned int tie_rs;
    // sum of the ranks of a particular tied value
  float tie_rank;
    // mean rank of a particular tied value (= tie_rs/to)

  float d;
    // deviation of rs2 from the expectation value exp_rs2
    // (or of rs1 from exp_rs1 if the test statistic is computed with
    // respect to the first sample -- default if w.r.t. the second
    // sample, but look at the end of the function in case it changed)
  float tc, sigma;
    // standard deviation and tie correction to this standard deviation

  std::list<int64_t>::iterator it1, it2;

  // Determine sample sizes and the sum since we need it various times

  n1 = sample1.size();
  n2 = sample2.size();
  n_sum = n1 + n2;
    
  // Sort samples

  sample1.sort();
  sample2.sort();
  outfile << "sorted sample old: " << sample1 << std::endl;
  outfile << "sorted sample new: " << sample2 << std::endl;

  // Calculate the sums of ranks for the two samples

  rs1 = 0; rs2 = 0; tc = 0;
  it1 = sample1.begin(); it2 = sample2.begin();

  for (unsigned int i = 1; i <= n_sum; i++) {

    outfile << i << ": ";

    if (*it1 == *it2) {

      outfile << *it1 << " tie! ";

      // special treatment for tie: assign mean rank
      tie = *it1;
      tie_rs = 0;
      to1 = 0; to2 = 0;

      // sum up all ranks for the current value:

      // in sample1
      do {
	tie_rs += i;
	to1++; // count number of occurences in sample1
	i++;
	it1++;
      }
      while ( (*it1 == tie) && (it1 != sample1.end()) );

      // in sample2
      do {
	tie_rs += i;
	to2++; // count number of occurences in sample2
	i++;
	it2++;
      }
      while ( (*it2 == tie) && (it2 != sample2.end()) );

      // we have to decrease i before next iteration begins,
      // as last loop increased it
      i--;

      // calculate the tie degree
      to = to1 + to2;
      // calculate mean rank
      tie_rank = float(tie_rs) / to;
      // print tie degrees and mean rank
      outfile << "to1=" << to1 << " to2=" << to2 << " meanrank=" <<tie_rank;
      // add mean rank to rs1 and rs2
      // as much as there are tie occurences in the samples
      rs1 += to1 * tie_rank;
      rs2 += to2 * tie_rank;

      // add value for sigma correction
      tc += to*to*to - to;

    }

    else if (*it1 < *it2) {

      outfile << *it1;
      // this rank belongs to sample1, thus we add it to the rank sum:
      rs1 += i;
      it1++;

    }

    else {

      outfile << *it2;
      // this rank belongs to sample2, thus we add it to the rank sum:
      rs2 += i;
      it2++;

    }

    outfile << " rs1=" << rs1 << " rs2=" << rs2 << std::endl;

    // check if sample1 has reached its end
    if (it1 == sample1.end()) {

      // all the remaining ranks belong to sample2
      outfile << "sample1 parsed\n";
      for (i++; i<=n_sum; i++) {
	// thus we add them to the rank sum
	rs2 += i;
	outfile << i << ": " << *it2
		<< " rs1=" << rs1 << " rs2=" << rs2 << std::endl;
	it2++;
      }

    }

    // check if sample2 has reached its end
    else if (it2 == sample2.end()) {

      // all the remaining ranks belong to sample1
      outfile << "sample2 parsed\n";
      for(i++; i<=n_sum; i++) {
	// thus we add them to the rank sum
	rs1 += i;
	outfile << i << ": " << *it1
		<< " rs1=" << rs1 << " rs2=" << rs2 << std::endl;
	it1++;
      }

    }

  }


  // When the null hypothesis H_0 is true:
  //
  // - expectation value (mean) of the random variable rsi is
  //     E[rsi] = ni*(n1+n2+1)/2          i=1,2
  // - variance of the random variable rsi is
  //     Var[rsi] = n1*n2*(n1+n2+1)/12    i=1,2
  //
  // When n1 and n2 are large enough, these random variable approximately
  // follow a normal distribution with mean mu = E[rsi] and standard
  // deviation sigma = sqrt(Var[rsi]).

  exp_rs1 = float(n1*(n_sum+1))/2;
  exp_rs2 = float(n2*(n_sum+1))/2;
  outfile << "exp. rs1=" << exp_rs1 << " exp. rs2=" << exp_rs2 << std::endl;

  // In fact we are more interested in the random variable rs2, as we
  // decided that our test statistic would be computed with respect to
  // the second sample.
  // In this case, we calculate the deviation d from the expectation value

  d = rs2 - exp_rs2;

  // Just in case you decide to compute the test statistic with respect
  // to the second sample, uncomment next line:
  //d = rs1 - exp_rs1;

  // Calculate sigma with tie correction according to Walter/Kendalls
  // (see Lothar Sachs: "Angewandte Statistik", 10th edition, p. 389)

  sigma = sqrt(float(n1*n2*(n_sum+1))/12 - tc/(12*(n_sum)*(n_sum-1)));
  outfile << "d=" << d << " sigma=" << sigma << std::endl;

  // Return p-value

  if (twosided == true) {

    if (d >= 0)
      return 2 * gsl_cdf_gaussian_Q (d, sigma);
    else
      return 2 * gsl_cdf_gaussian_P (d, sigma);
  }

  else // one-sided

    return gsl_cdf_gaussian_Q (d, sigma);

}
