
#include "open_char.h"

#include "Context.h"
#include "Template.h"
#include "Algorithms.h"

#include "TestUtils.h"


using namespace open_char;

void test_inv(Context &ctx, Algorithms &algs)
{
    CREATE_INV_CELL(ctx, c1)

    Template t("MY_TEMP");
    t.SetKind(TemplateKind::DELAY);

    t.AddIndex1(0.01);
    t.AddIndex1(0.1);

    t.AddIndex2(0.02);
    t.AddIndex2(0.3);

    c1.SetDelayTemplate(&t);

    // Logic table and logic function are precondition for combo delays
    RUN_SIMULATIONS(ctx, algs.PrepareComboLogicTableAndLeakageSims(c1));
    assert(algs.MeasureComboLogicTables(c1));
    algs.CalculateComboLogicFunctions(c1);

    RUN_SIMULATIONS(ctx, algs.PrepareComboDelayAndPowerSims(c1));
    assert(algs.MeasureComboDelays(c1));
    assert(algs.MeasureComboTransitions(c1));
    assert(algs.MeasureComboPowers(c1));

    Pin& pin = c1.GetPin("Y");

    Arc& arc = pin.GetArcs()[0];

    // cell_rise() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.111000, 0.356200"
    //     "0.124100, 0.372300"
    //     ) ;
    // } /* end cell_rise */

    EQUAL_WITH_TOL(arc.GetRiseDelays()[0][0], 0.111000);
    EQUAL_WITH_TOL(arc.GetRiseDelays()[0][1], 0.356200);
    EQUAL_WITH_TOL(arc.GetRiseDelays()[1][0], 0.124100);
    EQUAL_WITH_TOL(arc.GetRiseDelays()[1][1], 0.372300);

    // cell_fall() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.110600, 0.320000"
    //     "0.120700, 0.336000"
    //     ) ;
    // } /* end cell_fall */

    EQUAL_WITH_TOL(arc.GetFallDelays()[0][0], 0.110600);
    EQUAL_WITH_TOL(arc.GetFallDelays()[0][1], 0.320000);
    EQUAL_WITH_TOL(arc.GetFallDelays()[1][0], 0.120700);
    EQUAL_WITH_TOL(arc.GetFallDelays()[1][1], 0.336000);

    // rise_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.081800, 0.411300"
    //     "0.081700, 0.411300"
    //     ) ;
    // } /* end rise_transition */

    EQUAL_WITH_TOL(arc.GetRiseTransitions()[0][0], 0.081800);
    EQUAL_WITH_TOL(arc.GetRiseTransitions()[0][1], 0.411300);
    EQUAL_WITH_TOL(arc.GetRiseTransitions()[1][0], 0.081700);
    EQUAL_WITH_TOL(arc.GetRiseTransitions()[1][1], 0.411300);

    // fall_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.058700, 0.336300"
    //     "0.058900, 0.336500"
    //     ) ;
    // } /* end fall_transition */

    EQUAL_WITH_TOL(arc.GetFallTransitions()[0][0], 0.058700);
    EQUAL_WITH_TOL(arc.GetFallTransitions()[0][1], 0.336300);
    EQUAL_WITH_TOL(arc.GetFallTransitions()[1][0], 0.058900);
    EQUAL_WITH_TOL(arc.GetFallTransitions()[1][1], 0.336500);

    // rise_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "-0.042782, -0.441743"
    //     "-0.043125, -0.442354"
    //     ) ;
    // } /* end rise_power */

    EQUAL_WITH_TOL(arc.GetRisePowers()[0][0], 0.042782);
    EQUAL_WITH_TOL(arc.GetRisePowers()[0][1], 0.441743);
    EQUAL_WITH_TOL(arc.GetRisePowers()[1][0], 0.043125);
    EQUAL_WITH_TOL(arc.GetRisePowers()[1][1], 0.442354);

    // fall_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "-0.202559, -0.222347"
    //     "-0.190098, -0.212981"
    //     ) ;
    // } /* end fall_power */

    EQUAL_WITH_TOL(arc.GetFallPowers()[0][0], 0.202559);
    EQUAL_WITH_TOL(arc.GetFallPowers()[0][1], 0.222347);
    EQUAL_WITH_TOL(arc.GetFallPowers()[1][0], 0.190098);
    EQUAL_WITH_TOL(arc.GetFallPowers()[1][1], 0.212981);
}

void test_half_adder(Context &ctx, Algorithms &algs)
{
    CREATE_HALF_ADDER_CELL(ctx, c1);

    Template t("MY_TEMP");
    t.SetKind(TemplateKind::DELAY);

    t.AddIndex1(0.01);
    t.AddIndex1(0.1);

    t.AddIndex2(0.02);
    t.AddIndex2(0.3);

    c1.SetDelayTemplate(&t);

    // Logic table and logic function are precondition for combo delays
    RUN_SIMULATIONS(ctx, algs.PrepareComboLogicTableAndLeakageSims(c1));
    assert(algs.MeasureComboLogicTables(c1));
    algs.CalculateComboLogicFunctions(c1);

    // Measure Dealys
    RUN_SIMULATIONS(ctx, algs.PrepareComboDelayAndPowerSims(c1));
    assert(algs.MeasureComboDelays(c1));
    assert(algs.MeasureComboTransitions(c1));
    assert(algs.MeasureComboPowers(c1));

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

    EQUAL_WITH_TOL(co_b_arc.GetRiseDelays()[0][0], 0.107000);
    EQUAL_WITH_TOL(co_b_arc.GetRiseDelays()[0][1], 0.915000);
    EQUAL_WITH_TOL(co_b_arc.GetRiseDelays()[1][0], 0.111000);
    EQUAL_WITH_TOL(co_b_arc.GetRiseDelays()[1][1], 0.920000);

    // rise_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.102000, 1.385000"
    //     "0.103000, 1.385000"
    //     ) ;
    // } /* end rise_transition */

    EQUAL_WITH_TOL(co_b_arc.GetRiseTransitions()[0][0], 0.102000);
    EQUAL_WITH_TOL(co_b_arc.GetRiseTransitions()[0][1], 1.385000);
    EQUAL_WITH_TOL(co_b_arc.GetRiseTransitions()[1][0], 0.103000);
    EQUAL_WITH_TOL(co_b_arc.GetRiseTransitions()[1][1], 1.385000);

    // cell_fall() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.105000, 0.948000"
    //     "0.120000, 0.963000"
    //     ) ;
    // } /* end cell_fall */

    EQUAL_WITH_TOL(co_b_arc.GetFallDelays()[0][0], 0.105000);
    EQUAL_WITH_TOL(co_b_arc.GetFallDelays()[0][1], 0.948000);
    EQUAL_WITH_TOL(co_b_arc.GetFallDelays()[1][0], 0.120000);
    EQUAL_WITH_TOL(co_b_arc.GetFallDelays()[1][1], 0.963000);

    // fall_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.103000, 1.376000"
    //     "0.104000, 1.376000"
    //     ) ;
    // } /* end fall_transition */

    EQUAL_WITH_TOL(co_b_arc.GetFallTransitions()[0][0], 0.103000);
    EQUAL_WITH_TOL(co_b_arc.GetFallTransitions()[0][1], 1.376000);
    EQUAL_WITH_TOL(co_b_arc.GetFallTransitions()[1][0], 0.104000);
    EQUAL_WITH_TOL(co_b_arc.GetFallTransitions()[1][1], 1.376000);

    // rise_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.040429, 0.439443"
    //     "0.042533, 0.441363"
    //     ) ;
    // } /* end rise_power */

    EQUAL_WITH_TOL(co_b_arc.GetRisePowers()[0][0], 0.040429);
    EQUAL_WITH_TOL(co_b_arc.GetRisePowers()[0][1], 0.439443);
    EQUAL_WITH_TOL(co_b_arc.GetRisePowers()[1][0], 0.042533);
    EQUAL_WITH_TOL(co_b_arc.GetRisePowers()[1][1], 0.441363);

    //fall_power() {
    //    index_1 ("0.010000, 0.100000")
    //    index_2 ("0.020000, 0.300000")
    //    values (
    //    "0.009084, 0.008838"
    //    "0.009888, 0.009409"
    //    ) ;
    //} /* end fall_power */

    // TODO: Check Fall and Rise Powers are OK here, the numbers don't seem right
    EQUAL_WITH_TOL(co_b_arc.GetFallPowers()[0][0], 0.009084);
    EQUAL_WITH_TOL(co_b_arc.GetFallPowers()[0][1], 0.008838);
    EQUAL_WITH_TOL(co_b_arc.GetFallPowers()[1][0], 0.009888);
    EQUAL_WITH_TOL(co_b_arc.GetFallPowers()[1][1], 0.009409);

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

    EQUAL_WITH_TOL(co_a_arc.GetRiseDelays()[0][0], 0.102000);
    EQUAL_WITH_TOL(co_a_arc.GetRiseDelays()[0][1], 0.911000);
    EQUAL_WITH_TOL(co_a_arc.GetRiseDelays()[1][0], 0.110000);
    EQUAL_WITH_TOL(co_a_arc.GetRiseDelays()[1][1], 0.918000);

    // rise_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.103000, 1.384000"
    //     "0.105000, 1.385000"
    //     ) ;
    // } /* end rise_transition */

    EQUAL_WITH_TOL(co_a_arc.GetRiseTransitions()[0][0], 0.103000);
    EQUAL_WITH_TOL(co_a_arc.GetRiseTransitions()[0][1], 1.384000);
    EQUAL_WITH_TOL(co_a_arc.GetRiseTransitions()[1][0], 0.105000);
    EQUAL_WITH_TOL(co_a_arc.GetRiseTransitions()[1][1], 1.385000);

    // cell_fall() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.100000, 0.944000"
    //     "0.115000, 0.957000"
    //     ) ;
    // } /* end cell_fall */

    EQUAL_WITH_TOL(co_a_arc.GetFallDelays()[0][0], 0.100000);
    EQUAL_WITH_TOL(co_a_arc.GetFallDelays()[0][1], 0.944000);
    EQUAL_WITH_TOL(co_a_arc.GetFallDelays()[1][0], 0.115000);
    EQUAL_WITH_TOL(co_a_arc.GetFallDelays()[1][1], 0.957000);

    // fall_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.100000, 1.376000"
    //     "0.102000, 1.376000"
    //     ) ;
    // } /* end fall_transition */

    // TODO: Check 2nd and 4th being equal
    EQUAL_WITH_TOL(co_a_arc.GetFallTransitions()[0][0], 0.100000);
    EQUAL_WITH_TOL(co_a_arc.GetFallTransitions()[0][1], 1.376000);
    EQUAL_WITH_TOL(co_a_arc.GetFallTransitions()[1][0], 0.102000);
    EQUAL_WITH_TOL(co_a_arc.GetFallTransitions()[1][1], 1.376000);

    // rise_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.040200, 0.439251"
    //     "0.042166, 0.441069"
    //     ) ;
    // } /* end rise_power */

    EQUAL_WITH_TOL(co_a_arc.GetRisePowers()[0][0], 0.040200);
    EQUAL_WITH_TOL(co_a_arc.GetRisePowers()[0][1], 0.439251);
    EQUAL_WITH_TOL(co_a_arc.GetRisePowers()[1][0], 0.042166);
    EQUAL_WITH_TOL(co_a_arc.GetRisePowers()[1][1], 0.441069);

    // fall_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.009122, 0.008904"
    //     "0.010413, 0.009880"
    //     ) ;
    // } /* end fall_power */

    EQUAL_WITH_TOL(co_a_arc.GetFallPowers()[0][0], 0.009122);
    EQUAL_WITH_TOL(co_a_arc.GetFallPowers()[0][1], 0.008904);
    EQUAL_WITH_TOL(co_a_arc.GetFallPowers()[1][0], 0.010413);
    EQUAL_WITH_TOL(co_a_arc.GetFallPowers()[1][1], 0.009880);

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

    EQUAL_WITH_TOL(s_a_arc.GetRiseDelays()[0][0], 0.102000);
    EQUAL_WITH_TOL(s_a_arc.GetRiseDelays()[0][1], 0.911000);
    EQUAL_WITH_TOL(s_a_arc.GetRiseDelays()[1][0], 0.109000);
    EQUAL_WITH_TOL(s_a_arc.GetRiseDelays()[1][1], 0.917000);

    // rise_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.100000, 1.384000"
    //     "0.102000, 1.385000"
    //     ) ;
    // } /* end rise_transition */

    EQUAL_WITH_TOL(s_a_arc.GetRiseTransitions()[0][0], 0.100000);
    EQUAL_WITH_TOL(s_a_arc.GetRiseTransitions()[0][1], 1.384000);
    EQUAL_WITH_TOL(s_a_arc.GetRiseTransitions()[1][0], 0.102000);
    EQUAL_WITH_TOL(s_a_arc.GetRiseTransitions()[1][1], 1.385000);

    // cell_fall() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.105000, 0.948000"
    //     "0.115000, 0.958000"
    //     ) ;
    // } /* end cell_fall */

    EQUAL_WITH_TOL(s_a_arc.GetFallDelays()[0][0], 0.105000);
    EQUAL_WITH_TOL(s_a_arc.GetFallDelays()[0][1], 0.948000);
    EQUAL_WITH_TOL(s_a_arc.GetFallDelays()[1][0], 0.115000);
    EQUAL_WITH_TOL(s_a_arc.GetFallDelays()[1][1], 0.958000);

    // fall_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.102000, 1.376000"
    //     "0.101000, 1.377000"
    //     ) ;
    // } /* end fall_transition */

    EQUAL_WITH_TOL(s_a_arc.GetFallTransitions()[0][0], 0.102000);
    EQUAL_WITH_TOL(s_a_arc.GetFallTransitions()[0][1], 1.376000);
    EQUAL_WITH_TOL(s_a_arc.GetFallTransitions()[1][0], 0.101000);
    EQUAL_WITH_TOL(s_a_arc.GetFallTransitions()[1][1], 1.377000);

    // rise_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.034337, 0.433431"
    //     "0.035717, 0.434675"
    //     ) ;
    // } /* end rise_power */

    EQUAL_WITH_TOL(s_a_arc.GetRisePowers()[0][0], 0.034337);
    EQUAL_WITH_TOL(s_a_arc.GetRisePowers()[0][1], 0.433431);
    EQUAL_WITH_TOL(s_a_arc.GetRisePowers()[1][0], 0.035717);
    EQUAL_WITH_TOL(s_a_arc.GetRisePowers()[1][1], 0.434675);

    // fall_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.003953, 0.003903"
    //     "0.004238, 0.004004"
    //     ) ;
    // } /* end fall_power */

    EQUAL_WITH_TOL(s_a_arc.GetFallPowers()[0][0], 0.003953);
    EQUAL_WITH_TOL(s_a_arc.GetFallPowers()[0][1], 0.003903);
    EQUAL_WITH_TOL(s_a_arc.GetFallPowers()[1][0], 0.004238);
    EQUAL_WITH_TOL(s_a_arc.GetFallPowers()[1][1], 0.004004);

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

    EQUAL_WITH_TOL(s_b_arc.GetRiseDelays()[0][0], 0.111000);
    EQUAL_WITH_TOL(s_b_arc.GetRiseDelays()[0][1], 0.920000);
    EQUAL_WITH_TOL(s_b_arc.GetRiseDelays()[1][0], 0.117000);
    EQUAL_WITH_TOL(s_b_arc.GetRiseDelays()[1][1], 0.925000);

    // rise_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.101000, 1.385000"
    //     "0.104000, 1.385000"
    //     ) ;
    // } /* end rise_transition */

    EQUAL_WITH_TOL(s_b_arc.GetRiseTransitions()[0][0], 0.101000);
    EQUAL_WITH_TOL(s_b_arc.GetRiseTransitions()[0][1], 1.385000);
    EQUAL_WITH_TOL(s_b_arc.GetRiseTransitions()[1][0], 0.104000);
    EQUAL_WITH_TOL(s_b_arc.GetRiseTransitions()[1][1], 1.385000);

    // cell_fall() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.108000, 0.954000"
    //     "0.118000, 0.963000"
    //     ) ;
    // } /* end cell_fall */

    EQUAL_WITH_TOL(s_b_arc.GetFallDelays()[0][0], 0.108000);
    EQUAL_WITH_TOL(s_b_arc.GetFallDelays()[0][1], 0.954000);
    EQUAL_WITH_TOL(s_b_arc.GetFallDelays()[1][0], 0.118000);
    EQUAL_WITH_TOL(s_b_arc.GetFallDelays()[1][1], 0.963000);

    // fall_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.102000, 1.376000"
    //     "0.103000, 1.376000"
    //     ) ;
    // } /* end fall_transition */

    EQUAL_WITH_TOL(s_b_arc.GetFallTransitions()[0][0], 0.102000);
    EQUAL_WITH_TOL(s_b_arc.GetFallTransitions()[0][1], 1.376000);
    EQUAL_WITH_TOL(s_b_arc.GetFallTransitions()[1][0], 0.103000);
    EQUAL_WITH_TOL(s_b_arc.GetFallTransitions()[1][1], 1.376000);

    // rise_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.035031, 0.434159"
    //     "0.035943, 0.434978"
    //     ) ;
    // } /* end rise_power */

    EQUAL_WITH_TOL(s_b_arc.GetRisePowers()[0][0], 0.035031);
    EQUAL_WITH_TOL(s_b_arc.GetRisePowers()[0][1], 0.434159);
    EQUAL_WITH_TOL(s_b_arc.GetRisePowers()[1][0], 0.035943);
    EQUAL_WITH_TOL(s_b_arc.GetRisePowers()[1][1], 0.434978);

    // fall_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.004303, 0.004171"
    //     "0.004487, 0.004147"
    //     ) ;
    // } /* end fall_power */

    EQUAL_WITH_TOL(s_b_arc.GetFallPowers()[0][0], 0.004303);
    EQUAL_WITH_TOL(s_b_arc.GetFallPowers()[0][1], 0.004171);
    EQUAL_WITH_TOL(s_b_arc.GetFallPowers()[1][0], 0.004487);
    EQUAL_WITH_TOL(s_b_arc.GetFallPowers()[1][1], 0.004147);

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

    EQUAL_WITH_TOL(s_b_neg_arc.GetRiseDelays()[0][0], 0.133000);
    EQUAL_WITH_TOL(s_b_neg_arc.GetRiseDelays()[0][1], 0.941000);
    EQUAL_WITH_TOL(s_b_neg_arc.GetRiseDelays()[1][0], 0.148000);
    EQUAL_WITH_TOL(s_b_neg_arc.GetRiseDelays()[1][1], 0.957000);

    // rise_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.100000, 1.384000"
    //     "0.101000, 1.385000"
    //     ) ;
    // } /* end rise_transition */

    EQUAL_WITH_TOL(s_b_neg_arc.GetRiseTransitions()[0][0], 0.100000);
    EQUAL_WITH_TOL(s_b_neg_arc.GetRiseTransitions()[0][1], 1.384000);
    EQUAL_WITH_TOL(s_b_neg_arc.GetRiseTransitions()[1][0], 0.101000);
    EQUAL_WITH_TOL(s_b_neg_arc.GetRiseTransitions()[1][1], 1.385000);

    // cell_fall() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.139000, 0.982000"
    //     "0.145000, 0.990000"
    //     ) ;
    // } /* end cell_fall */

    EQUAL_WITH_TOL(s_b_neg_arc.GetFallDelays()[0][0], 0.139000);
    EQUAL_WITH_TOL(s_b_neg_arc.GetFallDelays()[0][1], 0.982000);
    EQUAL_WITH_TOL(s_b_neg_arc.GetFallDelays()[1][0], 0.145000);
    EQUAL_WITH_TOL(s_b_neg_arc.GetFallDelays()[1][1], 0.990000);

    // fall_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.100000, 1.375000"
    //     "0.099000, 1.375000"
    //     ) ;
    // } /* end fall_transition */

    EQUAL_WITH_TOL(s_b_neg_arc.GetFallTransitions()[0][0], 0.100000);
    EQUAL_WITH_TOL(s_b_neg_arc.GetFallTransitions()[0][1], 1.375000);
    EQUAL_WITH_TOL(s_b_neg_arc.GetFallTransitions()[1][0], 0.099000);
    EQUAL_WITH_TOL(s_b_neg_arc.GetFallTransitions()[1][1], 1.375000);

    // rise_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.037884, 0.436893"
    //     "0.039451, 0.438445"
    //     ) ;
    // } /* end rise_power */

    EQUAL_WITH_TOL(s_b_neg_arc.GetRisePowers()[0][0], 0.037884);
    EQUAL_WITH_TOL(s_b_neg_arc.GetRisePowers()[0][1], 0.436893);
    EQUAL_WITH_TOL(s_b_neg_arc.GetRisePowers()[1][0], 0.039451);
    EQUAL_WITH_TOL(s_b_neg_arc.GetRisePowers()[1][1], 0.438445);

    // fall_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.012134, 0.011989"
    //     "0.014315, 0.014079"
    //     ) ;
    // } /* end fall_power */

    EQUAL_WITH_TOL(s_b_neg_arc.GetFallPowers()[0][0], 0.012134);
    EQUAL_WITH_TOL(s_b_neg_arc.GetFallPowers()[0][1], 0.011989);
    EQUAL_WITH_TOL(s_b_neg_arc.GetFallPowers()[1][0], 0.014315);
    EQUAL_WITH_TOL(s_b_neg_arc.GetFallPowers()[1][1], 0.014079);

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

    EQUAL_WITH_TOL(s_a_neg_arc.GetRiseDelays()[0][0], 0.134000);
    EQUAL_WITH_TOL(s_a_neg_arc.GetRiseDelays()[0][1], 0.943000);
    EQUAL_WITH_TOL(s_a_neg_arc.GetRiseDelays()[1][0], 0.149000);
    EQUAL_WITH_TOL(s_a_neg_arc.GetRiseDelays()[1][1], 0.957000);

    // rise_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.103000, 1.385000"
    //     "0.103000, 1.386000"
    //     ) ;
    // } /* end rise_transition */

    EQUAL_WITH_TOL(s_a_neg_arc.GetRiseTransitions()[0][0], 0.103000);
    EQUAL_WITH_TOL(s_a_neg_arc.GetRiseTransitions()[0][1], 1.385000);
    EQUAL_WITH_TOL(s_a_neg_arc.GetRiseTransitions()[1][0], 0.103000);
    EQUAL_WITH_TOL(s_a_neg_arc.GetRiseTransitions()[1][1], 1.386000);

    // cell_fall() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.134000, 0.979000"
    //     "0.142000, 0.985000"
    //     ) ;
    // } /* end cell_fall */

    EQUAL_WITH_TOL(s_a_neg_arc.GetFallDelays()[0][0], 0.134000);
    EQUAL_WITH_TOL(s_a_neg_arc.GetFallDelays()[0][1], 0.979000);
    EQUAL_WITH_TOL(s_a_neg_arc.GetFallDelays()[1][0], 0.142000);
    EQUAL_WITH_TOL(s_a_neg_arc.GetFallDelays()[1][1], 0.985000);

    // fall_transition() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.100000, 1.375000"
    //     "0.100000, 1.375000"
    //     ) ;
    // } /* end fall_transition */

    EQUAL_WITH_TOL(s_a_neg_arc.GetFallTransitions()[0][0], 0.100000);
    EQUAL_WITH_TOL(s_a_neg_arc.GetFallTransitions()[0][1], 1.375000);
    EQUAL_WITH_TOL(s_a_neg_arc.GetFallTransitions()[1][0], 0.100000);
    EQUAL_WITH_TOL(s_a_neg_arc.GetFallTransitions()[1][1], 1.375000);

    // rise_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.037885, 0.436884"
    //     "0.040093, 0.439097"
    //     ) ;
    // } /* end rise_power */

    EQUAL_WITH_TOL(s_a_neg_arc.GetRisePowers()[0][0], 0.037885);
    EQUAL_WITH_TOL(s_a_neg_arc.GetRisePowers()[0][1], 0.436884);
    EQUAL_WITH_TOL(s_a_neg_arc.GetRisePowers()[1][0], 0.040093);
    EQUAL_WITH_TOL(s_a_neg_arc.GetRisePowers()[1][1], 0.439097);

    // fall_power() {
    //     index_1 ("0.010000, 0.100000")
    //     index_2 ("0.020000, 0.300000")
    //     values (
    //     "0.011259, 0.011032"
    //     "0.013447, 0.013117"
    //     ) ;
    // } /* end fall_power */

    EQUAL_WITH_TOL(s_a_neg_arc.GetFallPowers()[0][0], 0.011259);
    EQUAL_WITH_TOL(s_a_neg_arc.GetFallPowers()[0][1], 0.011032);
    EQUAL_WITH_TOL(s_a_neg_arc.GetFallPowers()[1][0], 0.013447);
    EQUAL_WITH_TOL(s_a_neg_arc.GetFallPowers()[1][1], 0.013117);
}


int main()
{
    ALG_TEST_INIT(ctx, algs);

    ctx.AddNetlist(TEST_NETLIST_DIR "/basic_gates.cdl");

    test_inv        (ctx, algs);
    test_half_adder (ctx, algs);

    return 0;
}