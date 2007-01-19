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

#include "pcs-test.h"
#include "shared.h"
#include <iostream>
#include <fstream>


/* PEARSON CHI-SQUARE TEST OF HOMOGENEITY
 *
 * This long explaination provides you with everything you must know
 * to understand totally Pearson chi-square test of homogeneity, and
 * the functions of this file. Its beginning is adapted from Morris
 * DeGroot "Probability and Statistics", third edition, ch. 9.4,
 * p. 556-558.
 *
 * We consider a problem in which random samples are taken from two
 * different populations (sample1 from population 1, sample2 from
 * population 2), and each observation in each sample can be
 * classified as one of noc different types (noc = number of
 * categories). If the observations in the samples are not already
 * classified in categories (for example, if the data is continuous,
 * or if they are all different, forming syngleton categories), we can
 * easily group them into categories: see function pcs_categories.
 *
 * We want to determine, on the basis of the observed values in the
 * samples, whether they come from the same distribution or not.
 *
 * The hypotheses to be tested are as follow:
 *
 *     H0: sample1 and sample2 are taken from distributions having
 *         the same distribution function
 *     H1: sample1 and sample2 are taken from distributions having
 *         different distribution functions
 *
 * For i=1,2 and j=1,...,noc, we let p_ij denote the probability that
 * an observation chosen at random from samplei will be of type j. The
 * hypopheses to be tested may be rewritten as:
 *
 *     H0: p_1j = p_2j for all j
 *     H1: H0 is not true
 *
 * i.e., we test the hypothesis that the two samples are drawn from
 * the same distribution against the hypothesis that they are not
 * (hence the name "test of homogeneity"). To do so, we must shape
 * a test statistic.
 *
 * For i=1,2 we let lgi denote the number of observations in samplei
 * (lg = length), and n denote the total number of observations,
 * i.e. n=lg1+lg2. For i=1,2 and j=1,...,noc we let N_ij denote the
 * number of observations in samplei that are of type j. For
 * j=1,...,noc we let N_j denote the total number of observations of
 * type j, without carring about which sample they are in.
 *
 * Suppose for the moment that the probabilities p_ij are known. We
 * consider the standard chi-square statistic for samplei:
 *
 *     sum_{j=1}^{noc} (N_ij - lgi*p_ij)^2 / lgi*p_ij
 *
 * When sample size lgi is large, the distribution of this statistic
 * is approximately a chi-square distribution with noc-1 degrees of
 * freedom.
 *
 * Our test statistic will be the sum of the previous statistic over
 * the two different samples:
 *
 *     Q = sum_{i=1}^2 sum_{j=1}^{noc} (N_ij - lgi*p_ij)^2 / lgi*p_ij
 *
 * Since the observations in the two samples are drawn independently,
 * our test statistic will also have approximately a chi-square
 * distribution, with 2*(noc-1) degrees of freedom.
 *
 * Yet the probabilities p_ij are not known. All we can do is
 * estimating their values from the observations in the two samples,
 * through the maximum likelihood estimator (M.L.E.) for instance.
 * When the null hypothesis H0 is true, the two samples are actually
 * drawn from the same distribution. Therefore the M.L.E. of the
 * probability p_ij is simply the frequency, the proportion of all the
 * observations of type j in the two samples:
 *
 *     M.L.E.(p_ij) = N_j / n
 *
 * (it doesn't depend from i). When replacing p_ij by its M.L.E. in
 * the statistic Q, we get:
 *
 *     Q = sum_{i=1}^2 sum_{j=1}^{noc} (N_ij - E_ij)^2 / E_ij
 *
 * where
 *
 *     E_ij = lgi*N_j / n
 *
 * Because the distributions of the two populations are the same when
 * H0 is true, and because sum_{j=1}^{noc} p_ij = 1, we have
 * estimated noc-1 parameters in this problem. Therefore, the
 * chi-square distribution of the statistic Q has only 2*(noc-1) -
 * (noc-1) = noc-1 degrees of freedom.
 *
 * This test statistic Q is a measure of the difference between the
 * observed values N_ij and the expected values E_ij of N_ij when the
 * null hypothesis is true. The bigger the difference, the bigger the
 * test statistic, and the clearer our impression that the two samples
 * are not drawn from the same distribution. Therefore our test
 * procedure can be written in the form:
 *
 *     reject H0 when Q >= c
 *
 * where c is a constant chosen such as to make the test have the
 * desired significance level alpha. Namely, c is the
 * (1-alpha)-quantile of Q: if we know alpha (traditionaly,
 * alpha=0.05) and are looking for the critical value c, we write that
 * the area on the right of c under the chi-square distribution of Q
 * must equal alpha, i.e. we write that Pr(Q >= c) = alpha, which
 * translates at once into Pr(Q <= c) = 1-alpha, i.e. c is the
 * (1-alpha)-quantile of Q. Therefore our test procedure should better
 * read:
 *
 *     reject H0 to significance level alpha when Q >= c
 *
 * But what if we did not chose alpha before running the test ? Or
 * what if we do not want to chose alpha, but are rather interested in
 * the smallest significance level to which we would reject H0,
 * considering, of course, the available data ? Rather than computing
 * the value q of Q and then compare q to c to raise a binary answer
 * (reject / don't reject), we could compute the p-value of the test
 * corresponding to the observed data, i.e. the smallest significance
 * level to which we would reject H0. That would provide us with more
 * information, as our program would say:
 *
 *     considering the data we have, reject H0
 *     to any significance level greater than
 *     the p-value of the data
 *
 * Depending of whether the p-value is low or high, we will have an
 * idea of how strong or weak is the evidence against H0 provided by
 * the data. For example, we will be much more comfortable with
 * rejecting H0 if the p-value is 0.0045 than if it is 0.0493. If we
 * sticked to the first approach (binary reject / don't reject
 * answer), we would reject H0 regardless of how close q is from c;
 * the second approach (p-value) enables us to be wiser about our
 * decision to reject H0 or not.
 *
 * For tests that can be written in the form "reject H0 when Q >= c",
 * the p-value of the test when the observed data yields a value q
 * for the test statistic Q is defined by observing that q is the
 * (1-p)-quantile of the distribution function of Q. Indeed, we said
 * previously that the critical value c was the (1-alpha)-quantile of Q,
 * and that p was the significance level alpha corresponding to the
 * critical value q. Hence the equation:
 *
 *     p = 1 - Q_cdf(q) = 1 - Pr(Q <= q)
 *
 * Q_cdf being the cumulative distribution function of Q,
 * i.e. approximately the c.d.f. of a random variable having
 * a chi-square distribution with noc-1 degrees of freedom.
 *
 * In the GNU Scientific Library, the function that computes the cdf in
 * x of a random variable X having a chi-square distribution with k
 * degrees of freedom is:
 *
 *     gsl_cdf_chisq_P(x,k) = Pr(X <= x)
 *
 * The GSL provides also the function:
 *
 *     gsl_cdf_chisq_Q(x,k) = Pr(X >= x) = 1 - Pr(X <= x)
 *
 * which gives us our p-value.
 *
 * Last thing to know: considering the expression and the
 * signification of the Q statistic (measure of the difference between
 * the observed values and their expected values when the null
 * hypothesis is true), we understand at once that it is a one-sided
 * test (i.e. the alternative hypothese is one-sided, we can indeed
 * re-write the hypotheses as: H0: Q < c and H1: Q >= c), and that,
 * contrary to what happens with Wilcoxon-Mann-Whitney and
 * Kolmogorov-Smirnov tests, any attempt to derive a corresponding
 * two-sided test from it does not make any sense.
 *
 * Thanks for the reading; now you know everything useful to
 * understand entirely the following two functions.
 */


/* This function distributes the values of the samples into categories
 * (intervals) and put the numbers of elements per categorie into the
 * vectors categories1 and categories2 (their size is therefore the
 * number of categories)
 * WARNING: categories1 and categories2 MUST be empty vectors!
 */
void pcs_categories ( std::list<int64_t> sample1, std::list<int64_t> sample2,
		      std::vector<unsigned> & categories1,
		      std::vector<unsigned> & categories2,
		      std::ofstream & outfile ) {

  sample1.sort(); outfile << "sorted sample1: " << sample1 << std::endl;
  sample2.sort(); outfile << "sorted sample2: " << sample2 << std::endl;

  unsigned lg1 = sample1.size();
  unsigned lg2 = sample2.size();
  unsigned n = lg1 + lg2;

  unsigned min = sample1.front()<sample2.front() ? sample1.front():sample2.front();
  unsigned sample1_oldsize = lg1;
  unsigned sample2_oldsize = lg2;
  double E1, E2;
  // M.L.E. of the probability that an item in sample1 (E1) and sample2 (E2)
  // will be in the current category, i.e. expected number of observations
  // (of items, of elements) in the current category for both samples;
  // the approximation is satisfactory when M.L.E. >= 1.5 for each category,
  // and very good when >= 5
  unsigned n1, n2;
  // number of elements in the current category for sample1 and sample2

  while (true) {

    E1 = E2 = 0.;
    n1 = n2 = 0;

    while (E1 < 5 || E2 < 5) {

      sample1.remove(min);
      sample2.remove(min);

      n1 += sample1_oldsize - sample1.size();
        // number of elements = min in sample1
      n2 += sample2_oldsize - sample2.size();
        // number of elements = min in sample2

      if (sample1.empty() == true || sample2.empty() == true) goto end;

      E1 = double(lg1 * (n1+n2)) / n;
      E2 = double(lg2 * (n1+n2)) / n;

      min = sample1.front()<sample2.front() ? sample1.front():sample2.front();
      sample1_oldsize = sample1.size();
      sample2_oldsize = sample2.size();

    }

    categories1.push_back(n1);
    categories2.push_back(n2);

  }

  end : {

    if (sample1.empty() == true && sample2.empty() == false)
      n2 += sample2.size();
    if (sample1.empty() == false && sample2.empty() == true)
      n1 += sample1.size();

    categories1.push_back(n1);
    categories2.push_back(n2);

  }

  outfile << "categories1: " << categories1 << std::endl;
  outfile << "categories2: " << categories2 << std::endl;

  return;

}

/* This function returns an approximated p-value of Pearson chi-square test.
 *
 * The hypothesis to be tested are:
 *
 *     H0: the samples are drawn from populations having same distribution
 *     H1: they come from populations with different distributions
 *
 * The test statistic is
 *
 *     Q = sum_{i=1}^2 sum_{j=1}^{noc} (N_ij - E_ij)^2 / E_ij
 *
 * (notations from the text at the begining of this file).
 *
 * The p-value when the observed value for Q is q is computed through:
 *
 *     p = 1 - Pr(Q <= q) = Pr(Q >= q) = gsl_cdf_chisq_Q (q)
 *
 * We reject H0 to significance level alpha if p-value < alpha,
 * i.e. considering the data we have, we reject H0 to any significance
 * level alpha > p-value. (You can find more information in the text
 * at the beginning of this file.)
 *
 * As explained in this text, Pearson chi-square test is one-sided and
 * should not be put into a two-sided form; it would not make sense.
 */
double pcs_test ( const std::list<int64_t> & sample1,
		  const std::list<int64_t> & sample2,
		  std::ofstream & outfile ) {

  std::vector<unsigned> categories1;
  std::vector<unsigned> categories2;
  pcs_categories (sample1, sample2, categories1, categories2, outfile);

  unsigned lg1 = sample1.size();
  unsigned lg2 = sample2.size();
  unsigned n = lg1 + lg2;
  unsigned noc = categories1.size();
    // = categories2.size(): number of categories

  double q = 0;
  for (unsigned i = 0; i < noc; i++) {

    double E1 = double(lg1 * (categories1[i]+categories2[i])) / n;
    double E2 = double(lg2 * (categories1[i]+categories2[i])) / n;

    double d1 = (categories1[i] - E1);
    double d2 = (categories2[i] - E2);

    q += d1*d1/E1 + d2*d2/E2;
      // q is the observed value of the chi-square statistic Q

  }

  // return p-value
  return gsl_cdf_chisq_Q(q, noc-1); // = Pr(Q >= q) = 1 - Pr(Q <= q)

}
