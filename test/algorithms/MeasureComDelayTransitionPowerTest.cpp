
#include "open_char.h"

#include "Context.h"
#include "Template.h"
#include "Algorithms.h"

#include "TestUtils.h"


using namespace open_char;

static void test_inv(Context &ctx, Algorithms &algs)
{
    CREATE_INV_CELL(ctx, c1)

    Template t("MY_TEMP");
    t.SetKind(TemplKind::DELAY);

    t.AddIndex1(0.01);
    t.AddIndex1(0.1);

    t.AddIndex2(0.02);
    t.AddIndex2(0.3);

    c1.SetDelayTemplate(&t);

    // Logic table and logic function are precondition for combo delays
    RUN_SIMULATIONS(ctx, algs.PrepareComLogicTablesSims(c1));
    assert(algs.MeasureComLogicTables(c1));
    algs.CalculateComLogicFunctions(c1);

    RUN_SIMULATIONS(ctx, algs.PrepareComDelayTransitionPowerSims(c1));
    assert(algs.MeasureComDelaysTransitionsPowers(c1));

    Pin& pin = c1.GetPin("Y");

    Arc& arc = pin.GetArcs()[0];

    // cell_rise() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //          "0.111000, 0.355000"
    //          "0.125000, 0.373000"
    //     ) ;
    // } /* end cell_rise */

    CHECK_FLOAT(arc.GetRiseDelays()[0][0], 0.111000);
    CHECK_FLOAT(arc.GetRiseDelays()[0][1], 0.355000);
    CHECK_FLOAT(arc.GetRiseDelays()[1][0], 0.125000);
    CHECK_FLOAT(arc.GetRiseDelays()[1][1], 0.373000);

    // rise_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //        "0.081000, 0.410000"
    //        "0.081000, 0.411000"
    //     ) ;
    // } /* end rise_transition */

    CHECK_FLOAT(arc.GetRiseTransitions()[0][0], 0.081800);
    CHECK_FLOAT(arc.GetRiseTransitions()[0][1], 0.410000);
    CHECK_FLOAT(arc.GetRiseTransitions()[1][0], 0.081000);
    CHECK_FLOAT(arc.GetRiseTransitions()[1][1], 0.411000);

    // cell_fall() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //       "0.111000, 0.320000"
    //       "0.122000, 0.336000"
    //     ) ;
    // } /* end cell_fall */

    CHECK_FLOAT(arc.GetFallDelays()[0][0], 0.111000);
    CHECK_FLOAT(arc.GetFallDelays()[0][1], 0.320000);
    CHECK_FLOAT(arc.GetFallDelays()[1][0], 0.122000);
    CHECK_FLOAT(arc.GetFallDelays()[1][1], 0.336000);

    // fall_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //          "0.057000, 0.336000"
    //          "0.058000, 0.336000"
    //     ) ;
    // } /* end fall_transition */

    CHECK_FLOAT(arc.GetFallTransitions()[0][0], 0.057000);
    CHECK_FLOAT(arc.GetFallTransitions()[0][1], 0.336000);
    CHECK_FLOAT(arc.GetFallTransitions()[1][0], 0.058000);
    CHECK_FLOAT(arc.GetFallTransitions()[1][1], 0.336000);

    // rise_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //          "0.040743, 0.433483"
    //          "0.043632, 0.444537"
    //     ) ;
    // } /* end rise_power */

    CHECK_FLOAT(arc.GetRisePowers()[0][0], 0.040743);
    CHECK_FLOAT(arc.GetRisePowers()[0][1], 0.433483);
    CHECK_FLOAT(arc.GetRisePowers()[1][0], 0.043632);
    CHECK_FLOAT(arc.GetRisePowers()[1][1], 0.444537);

    // fall_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //          "0.216491, 0.240236"
    //          "0.192435, 0.214416"
    //     ) ;
    // } /* end fall_power */

    CHECK_FLOAT(arc.GetFallPowers()[0][0], 0.216491);
    CHECK_FLOAT(arc.GetFallPowers()[0][1], 0.240236);
    CHECK_FLOAT(arc.GetFallPowers()[1][0], 0.192435);
    CHECK_FLOAT(arc.GetFallPowers()[1][1], 0.214416);
}

static void test_half_adder(Context &ctx, Algorithms &algs)
{
    CREATE_HALF_ADDER_CELL(ctx, c1);

    Template t("MY_TEMP");
    t.SetKind(TemplKind::DELAY);

    t.AddIndex1(0.01);
    t.AddIndex1(0.1);

    t.AddIndex2(0.02);
    t.AddIndex2(0.3);

    c1.SetDelayTemplate(&t);

    // Logic table and logic function are precondition for combo delays
    RUN_SIMULATIONS(ctx, algs.PrepareComLogicTablesSims(c1));
    assert(algs.MeasureComLogicTables(c1));
    algs.CalculateComLogicFunctions(c1);

    // Measure Dealys
    RUN_SIMULATIONS(ctx, algs.PrepareComDelayTransitionPowerSims(c1));
    assert(algs.MeasureComDelaysTransitionsPowers(c1));

    Pin& co_pin = c1.GetPin("CO");
    Arc& co_b_arc = co_pin.GetArcs()[0];

    assert(co_b_arc.GetRelatedPin() == &(c1.GetPin("B")));

    ///////////////////////////////////////////////////////////////////////////
    // B -> CO
    ///////////////////////////////////////////////////////////////////////////

    // cell_rise() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.107000, 0.915000"
    //     "0.111000, 0.920000"
    //     ) ;
    // } /* end cell_rise */

    CHECK_FLOAT(co_b_arc.GetRiseDelays()[0][0], 0.107000);
    CHECK_FLOAT(co_b_arc.GetRiseDelays()[0][1], 0.915000);
    CHECK_FLOAT(co_b_arc.GetRiseDelays()[1][0], 0.111000);
    CHECK_FLOAT(co_b_arc.GetRiseDelays()[1][1], 0.920000);

    // rise_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.102000, 1.385000"
    //     "0.103000, 1.385000"
    //     ) ;
    // } /* end rise_transition */

    CHECK_FLOAT(co_b_arc.GetRiseTransitions()[0][0], 0.102000);
    CHECK_FLOAT(co_b_arc.GetRiseTransitions()[0][1], 1.385000);
    CHECK_FLOAT(co_b_arc.GetRiseTransitions()[1][0], 0.103000);
    CHECK_FLOAT(co_b_arc.GetRiseTransitions()[1][1], 1.385000);

    // cell_fall() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.105000, 0.948000"
    //     "0.120000, 0.963000"
    //     ) ;
    // } /* end cell_fall */

    CHECK_FLOAT(co_b_arc.GetFallDelays()[0][0], 0.105000);
    CHECK_FLOAT(co_b_arc.GetFallDelays()[0][1], 0.948000);
    CHECK_FLOAT(co_b_arc.GetFallDelays()[1][0], 0.120000);
    CHECK_FLOAT(co_b_arc.GetFallDelays()[1][1], 0.963000);

    // fall_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.103000, 1.376000"
    //     "0.104000, 1.376000"
    //     ) ;
    // } /* end fall_transition */

    CHECK_FLOAT(co_b_arc.GetFallTransitions()[0][0], 0.103000);
    CHECK_FLOAT(co_b_arc.GetFallTransitions()[0][1], 1.376000);
    CHECK_FLOAT(co_b_arc.GetFallTransitions()[1][0], 0.104000);
    CHECK_FLOAT(co_b_arc.GetFallTransitions()[1][1], 1.376000);

    // rise_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.040429, 0.439443"
    //     "0.042533, 0.441363"
    //     ) ;
    // } /* end rise_power */

    CHECK_FLOAT(co_b_arc.GetRisePowers()[0][0], 0.040429);
    CHECK_FLOAT(co_b_arc.GetRisePowers()[0][1], 0.439443);
    CHECK_FLOAT(co_b_arc.GetRisePowers()[1][0], 0.042533);
    CHECK_FLOAT(co_b_arc.GetRisePowers()[1][1], 0.441363);

    //fall_power() {
    //    index_1 ("0.010000, 0.100000")
    //    index_2 ("0.020000, 0.300000")
    //    values (
    //    "0.009084, 0.008838"
    //    "0.009888, 0.009409"
    //    ) ;
    //} /* end fall_power */

    // TODO: Check Fall and Rise Powers are OK here, the numbers don't seem right
    CHECK_FLOAT(co_b_arc.GetFallPowers()[0][0], 0.009084);
    CHECK_FLOAT(co_b_arc.GetFallPowers()[0][1], 0.008838);
    CHECK_FLOAT(co_b_arc.GetFallPowers()[1][0], 0.009888);
    CHECK_FLOAT(co_b_arc.GetFallPowers()[1][1], 0.009409);

    ///////////////////////////////////////////////////////////////////////////
    // A -> CO
    ///////////////////////////////////////////////////////////////////////////

    Arc& co_a_arc = co_pin.GetArcs()[1];
    assert(co_a_arc.GetRelatedPin() == &(c1.GetPin("A")));

    // cell_rise() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.102000, 0.911000"
    //     "0.110000, 0.918000"
    //     ) ;
    // } /* end cell_rise */

    CHECK_FLOAT(co_a_arc.GetRiseDelays()[0][0], 0.102000);
    CHECK_FLOAT(co_a_arc.GetRiseDelays()[0][1], 0.911000);
    CHECK_FLOAT(co_a_arc.GetRiseDelays()[1][0], 0.110000);
    CHECK_FLOAT(co_a_arc.GetRiseDelays()[1][1], 0.918000);

    // rise_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.103000, 1.384000"
    //     "0.105000, 1.385000"
    //     ) ;
    // } /* end rise_transition */

    CHECK_FLOAT(co_a_arc.GetRiseTransitions()[0][0], 0.103000);
    CHECK_FLOAT(co_a_arc.GetRiseTransitions()[0][1], 1.384000);
    CHECK_FLOAT(co_a_arc.GetRiseTransitions()[1][0], 0.105000);
    CHECK_FLOAT(co_a_arc.GetRiseTransitions()[1][1], 1.385000);

    // cell_fall() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.100000, 0.944000"
    //     "0.115000, 0.957000"
    //     ) ;
    // } /* end cell_fall */

    CHECK_FLOAT(co_a_arc.GetFallDelays()[0][0], 0.100000);
    CHECK_FLOAT(co_a_arc.GetFallDelays()[0][1], 0.944000);
    CHECK_FLOAT(co_a_arc.GetFallDelays()[1][0], 0.115000);
    CHECK_FLOAT(co_a_arc.GetFallDelays()[1][1], 0.957000);

    // fall_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.100000, 1.376000"
    //     "0.102000, 1.376000"
    //     ) ;
    // } /* end fall_transition */

    // TODO: Check 2nd and 4th being equal
    CHECK_FLOAT(co_a_arc.GetFallTransitions()[0][0], 0.100000);
    CHECK_FLOAT(co_a_arc.GetFallTransitions()[0][1], 1.376000);
    CHECK_FLOAT(co_a_arc.GetFallTransitions()[1][0], 0.102000);
    CHECK_FLOAT(co_a_arc.GetFallTransitions()[1][1], 1.376000);

    // rise_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.040200, 0.439251"
    //     "0.042166, 0.441069"
    //     ) ;
    // } /* end rise_power */

    CHECK_FLOAT(co_a_arc.GetRisePowers()[0][0], 0.040200);
    CHECK_FLOAT(co_a_arc.GetRisePowers()[0][1], 0.439251);
    CHECK_FLOAT(co_a_arc.GetRisePowers()[1][0], 0.042166);
    CHECK_FLOAT(co_a_arc.GetRisePowers()[1][1], 0.441069);

    // fall_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.009122, 0.008904"
    //     "0.010413, 0.009880"
    //     ) ;
    // } /* end fall_power */

    CHECK_FLOAT(co_a_arc.GetFallPowers()[0][0], 0.009122);
    CHECK_FLOAT(co_a_arc.GetFallPowers()[0][1], 0.008904);
    CHECK_FLOAT(co_a_arc.GetFallPowers()[1][0], 0.010413);
    CHECK_FLOAT(co_a_arc.GetFallPowers()[1][1], 0.009880);

    ///////////////////////////////////////////////////////////////////////////
    // A -> S (positive_unate)
    ///////////////////////////////////////////////////////////////////////////
    Pin& s_pin = c1.GetPin("S");
    Arc& s_a_arc = s_pin.GetArcs()[0];
    assert(s_a_arc.GetRelatedPin() == &(c1.GetPin("A")));

    // cell_rise() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.102000, 0.911000"
    //     "0.109000, 0.917000"
    //     ) ;
    // } /* end cell_rise */

    CHECK_FLOAT(s_a_arc.GetRiseDelays()[0][0], 0.102000);
    CHECK_FLOAT(s_a_arc.GetRiseDelays()[0][1], 0.911000);
    CHECK_FLOAT(s_a_arc.GetRiseDelays()[1][0], 0.109000);
    CHECK_FLOAT(s_a_arc.GetRiseDelays()[1][1], 0.917000);

    // rise_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.100000, 1.384000"
    //     "0.102000, 1.385000"
    //     ) ;
    // } /* end rise_transition */

    CHECK_FLOAT(s_a_arc.GetRiseTransitions()[0][0], 0.100000);
    CHECK_FLOAT(s_a_arc.GetRiseTransitions()[0][1], 1.384000);
    CHECK_FLOAT(s_a_arc.GetRiseTransitions()[1][0], 0.102000);
    CHECK_FLOAT(s_a_arc.GetRiseTransitions()[1][1], 1.385000);

    // cell_fall() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.105000, 0.948000"
    //     "0.115000, 0.958000"
    //     ) ;
    // } /* end cell_fall */

    CHECK_FLOAT(s_a_arc.GetFallDelays()[0][0], 0.105000);
    CHECK_FLOAT(s_a_arc.GetFallDelays()[0][1], 0.948000);
    CHECK_FLOAT(s_a_arc.GetFallDelays()[1][0], 0.115000);
    CHECK_FLOAT(s_a_arc.GetFallDelays()[1][1], 0.958000);

    // fall_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.102000, 1.376000"
    //     "0.101000, 1.377000"
    //     ) ;
    // } /* end fall_transition */

    CHECK_FLOAT(s_a_arc.GetFallTransitions()[0][0], 0.102000);
    CHECK_FLOAT(s_a_arc.GetFallTransitions()[0][1], 1.376000);
    CHECK_FLOAT(s_a_arc.GetFallTransitions()[1][0], 0.101000);
    CHECK_FLOAT(s_a_arc.GetFallTransitions()[1][1], 1.377000);

    // rise_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.034337, 0.433431"
    //     "0.035717, 0.434675"
    //     ) ;
    // } /* end rise_power */

    CHECK_FLOAT(s_a_arc.GetRisePowers()[0][0], 0.034337);
    CHECK_FLOAT(s_a_arc.GetRisePowers()[0][1], 0.433431);
    CHECK_FLOAT(s_a_arc.GetRisePowers()[1][0], 0.035717);
    CHECK_FLOAT(s_a_arc.GetRisePowers()[1][1], 0.434675);

    // fall_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.003953, 0.003903"
    //     "0.004238, 0.004004"
    //     ) ;
    // } /* end fall_power */

    CHECK_FLOAT(s_a_arc.GetFallPowers()[0][0], 0.003953);
    CHECK_FLOAT(s_a_arc.GetFallPowers()[0][1], 0.003903);
    CHECK_FLOAT(s_a_arc.GetFallPowers()[1][0], 0.004238);
    CHECK_FLOAT(s_a_arc.GetFallPowers()[1][1], 0.004004);

    ///////////////////////////////////////////////////////////////////////////
    // B -> S (positive_unate)
    ///////////////////////////////////////////////////////////////////////////
    Arc& s_b_arc = s_pin.GetArcs()[1];
    assert(s_b_arc.GetRelatedPin() == &(c1.GetPin("B")));

    // cell_rise() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.111000, 0.920000"
    //     "0.117000, 0.925000"
    //     ) ;
    // } /* end cell_rise */

    CHECK_FLOAT(s_b_arc.GetRiseDelays()[0][0], 0.111000);
    CHECK_FLOAT(s_b_arc.GetRiseDelays()[0][1], 0.920000);
    CHECK_FLOAT(s_b_arc.GetRiseDelays()[1][0], 0.117000);
    CHECK_FLOAT(s_b_arc.GetRiseDelays()[1][1], 0.925000);

    // rise_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.101000, 1.385000"
    //     "0.104000, 1.385000"
    //     ) ;
    // } /* end rise_transition */

    CHECK_FLOAT(s_b_arc.GetRiseTransitions()[0][0], 0.101000);
    CHECK_FLOAT(s_b_arc.GetRiseTransitions()[0][1], 1.385000);
    CHECK_FLOAT(s_b_arc.GetRiseTransitions()[1][0], 0.104000);
    CHECK_FLOAT(s_b_arc.GetRiseTransitions()[1][1], 1.385000);

    // cell_fall() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.108000, 0.954000"
    //     "0.118000, 0.963000"
    //     ) ;
    // } /* end cell_fall */

    CHECK_FLOAT(s_b_arc.GetFallDelays()[0][0], 0.108000);
    CHECK_FLOAT(s_b_arc.GetFallDelays()[0][1], 0.954000);
    CHECK_FLOAT(s_b_arc.GetFallDelays()[1][0], 0.118000);
    CHECK_FLOAT(s_b_arc.GetFallDelays()[1][1], 0.963000);

    // fall_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.102000, 1.376000"
    //     "0.103000, 1.376000"
    //     ) ;
    // } /* end fall_transition */

    CHECK_FLOAT(s_b_arc.GetFallTransitions()[0][0], 0.102000);
    CHECK_FLOAT(s_b_arc.GetFallTransitions()[0][1], 1.376000);
    CHECK_FLOAT(s_b_arc.GetFallTransitions()[1][0], 0.103000);
    CHECK_FLOAT(s_b_arc.GetFallTransitions()[1][1], 1.376000);

    // rise_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.035031, 0.434159"
    //     "0.035943, 0.434978"
    //     ) ;
    // } /* end rise_power */

    CHECK_FLOAT(s_b_arc.GetRisePowers()[0][0], 0.035031);
    CHECK_FLOAT(s_b_arc.GetRisePowers()[0][1], 0.434159);
    CHECK_FLOAT(s_b_arc.GetRisePowers()[1][0], 0.035943);
    CHECK_FLOAT(s_b_arc.GetRisePowers()[1][1], 0.434978);

    // fall_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.004303, 0.004171"
    //     "0.004487, 0.004147"
    //     ) ;
    // } /* end fall_power */

    CHECK_FLOAT(s_b_arc.GetFallPowers()[0][0], 0.004303);
    CHECK_FLOAT(s_b_arc.GetFallPowers()[0][1], 0.004171);
    CHECK_FLOAT(s_b_arc.GetFallPowers()[1][0], 0.004487);
    CHECK_FLOAT(s_b_arc.GetFallPowers()[1][1], 0.004147);

    ///////////////////////////////////////////////////////////////////////////
    // B -> S (negative_unate)
    ///////////////////////////////////////////////////////////////////////////
    Arc& s_b_neg_arc = s_pin.GetArcs()[2];
    assert(s_b_neg_arc.GetRelatedPin() == &(c1.GetPin("B")));

    // cell_rise() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.133000, 0.941000"
    //     "0.148000, 0.957000"
    //     ) ;
    // } /* end cell_rise */

    CHECK_FLOAT(s_b_neg_arc.GetRiseDelays()[0][0], 0.133000);
    CHECK_FLOAT(s_b_neg_arc.GetRiseDelays()[0][1], 0.941000);
    CHECK_FLOAT(s_b_neg_arc.GetRiseDelays()[1][0], 0.148000);
    CHECK_FLOAT(s_b_neg_arc.GetRiseDelays()[1][1], 0.957000);

    // rise_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.100000, 1.384000"
    //     "0.101000, 1.385000"
    //     ) ;
    // } /* end rise_transition */

    CHECK_FLOAT(s_b_neg_arc.GetRiseTransitions()[0][0], 0.100000);
    CHECK_FLOAT(s_b_neg_arc.GetRiseTransitions()[0][1], 1.384000);
    CHECK_FLOAT(s_b_neg_arc.GetRiseTransitions()[1][0], 0.101000);
    CHECK_FLOAT(s_b_neg_arc.GetRiseTransitions()[1][1], 1.385000);

    // cell_fall() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.139000, 0.982000"
    //     "0.145000, 0.990000"
    //     ) ;
    // } /* end cell_fall */

    CHECK_FLOAT(s_b_neg_arc.GetFallDelays()[0][0], 0.139000);
    CHECK_FLOAT(s_b_neg_arc.GetFallDelays()[0][1], 0.982000);
    CHECK_FLOAT(s_b_neg_arc.GetFallDelays()[1][0], 0.145000);
    CHECK_FLOAT(s_b_neg_arc.GetFallDelays()[1][1], 0.990000);

    // fall_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.100000, 1.375000"
    //     "0.099000, 1.375000"
    //     ) ;
    // } /* end fall_transition */

    CHECK_FLOAT(s_b_neg_arc.GetFallTransitions()[0][0], 0.100000);
    CHECK_FLOAT(s_b_neg_arc.GetFallTransitions()[0][1], 1.375000);
    CHECK_FLOAT(s_b_neg_arc.GetFallTransitions()[1][0], 0.099000);
    CHECK_FLOAT(s_b_neg_arc.GetFallTransitions()[1][1], 1.375000);

    // rise_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.037884, 0.436893"
    //     "0.039451, 0.438445"
    //     ) ;
    // } /* end rise_power */

    CHECK_FLOAT(s_b_neg_arc.GetRisePowers()[0][0], 0.037884);
    CHECK_FLOAT(s_b_neg_arc.GetRisePowers()[0][1], 0.436893);
    CHECK_FLOAT(s_b_neg_arc.GetRisePowers()[1][0], 0.039451);
    CHECK_FLOAT(s_b_neg_arc.GetRisePowers()[1][1], 0.438445);

    // fall_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.012134, 0.011989"
    //     "0.014315, 0.014079"
    //     ) ;
    // } /* end fall_power */

    CHECK_FLOAT(s_b_neg_arc.GetFallPowers()[0][0], 0.012134);
    CHECK_FLOAT(s_b_neg_arc.GetFallPowers()[0][1], 0.011989);
    CHECK_FLOAT(s_b_neg_arc.GetFallPowers()[1][0], 0.014315);
    CHECK_FLOAT(s_b_neg_arc.GetFallPowers()[1][1], 0.014079);

    ///////////////////////////////////////////////////////////////////////////
    // A -> S (negative_unate)
    ///////////////////////////////////////////////////////////////////////////
    Arc& s_a_neg_arc = s_pin.GetArcs()[3];
    assert(s_a_neg_arc.GetRelatedPin() == &(c1.GetPin("A")));

    // cell_rise() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.134000, 0.943000"
    //     "0.149000, 0.957000"
    //     ) ;
    // } /* end cell_rise */

    CHECK_FLOAT(s_a_neg_arc.GetRiseDelays()[0][0], 0.134000);
    CHECK_FLOAT(s_a_neg_arc.GetRiseDelays()[0][1], 0.943000);
    CHECK_FLOAT(s_a_neg_arc.GetRiseDelays()[1][0], 0.149000);
    CHECK_FLOAT(s_a_neg_arc.GetRiseDelays()[1][1], 0.957000);

    // rise_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.103000, 1.385000"
    //     "0.103000, 1.386000"
    //     ) ;
    // } /* end rise_transition */

    CHECK_FLOAT(s_a_neg_arc.GetRiseTransitions()[0][0], 0.103000);
    CHECK_FLOAT(s_a_neg_arc.GetRiseTransitions()[0][1], 1.385000);
    CHECK_FLOAT(s_a_neg_arc.GetRiseTransitions()[1][0], 0.103000);
    CHECK_FLOAT(s_a_neg_arc.GetRiseTransitions()[1][1], 1.386000);

    // cell_fall() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.134000, 0.979000"
    //     "0.142000, 0.985000"
    //     ) ;
    // } /* end cell_fall */

    CHECK_FLOAT(s_a_neg_arc.GetFallDelays()[0][0], 0.134000);
    CHECK_FLOAT(s_a_neg_arc.GetFallDelays()[0][1], 0.979000);
    CHECK_FLOAT(s_a_neg_arc.GetFallDelays()[1][0], 0.142000);
    CHECK_FLOAT(s_a_neg_arc.GetFallDelays()[1][1], 0.985000);

    // fall_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.100000, 1.375000"
    //     "0.100000, 1.375000"
    //     ) ;
    // } /* end fall_transition */

    CHECK_FLOAT(s_a_neg_arc.GetFallTransitions()[0][0], 0.100000);
    CHECK_FLOAT(s_a_neg_arc.GetFallTransitions()[0][1], 1.375000);
    CHECK_FLOAT(s_a_neg_arc.GetFallTransitions()[1][0], 0.100000);
    CHECK_FLOAT(s_a_neg_arc.GetFallTransitions()[1][1], 1.375000);

    // rise_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.037885, 0.436884"
    //     "0.040093, 0.439097"
    //     ) ;
    // } /* end rise_power */

    CHECK_FLOAT(s_a_neg_arc.GetRisePowers()[0][0], 0.037885);
    CHECK_FLOAT(s_a_neg_arc.GetRisePowers()[0][1], 0.436884);
    CHECK_FLOAT(s_a_neg_arc.GetRisePowers()[1][0], 0.040093);
    CHECK_FLOAT(s_a_neg_arc.GetRisePowers()[1][1], 0.439097);

    // fall_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.011259, 0.011032"
    //     "0.013447, 0.013117"
    //     ) ;
    // } /* end fall_power */

    CHECK_FLOAT(s_a_neg_arc.GetFallPowers()[0][0], 0.011259);
    CHECK_FLOAT(s_a_neg_arc.GetFallPowers()[0][1], 0.011032);
    CHECK_FLOAT(s_a_neg_arc.GetFallPowers()[1][0], 0.013447);
    CHECK_FLOAT(s_a_neg_arc.GetFallPowers()[1][1], 0.013117);
}


int main()
{
    ALG_TEST_INIT(ctx, algs);

    ctx.AddNetlist(TEST_NETLIST_DIR "/basic_gates.cdl");

    test_inv        (ctx, algs);
    test_half_adder (ctx, algs);

    return 0;
}