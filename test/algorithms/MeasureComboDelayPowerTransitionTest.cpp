
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
    algs.MeasureComboLogicTables(c1);
    algs.CalculateComboLogicFunctions(c1);

    RUN_SIMULATIONS(ctx, algs.PrepareComboDelayAndPowerSims(c1));
    algs.MeasureComboDelays(c1);
    algs.MeasureComboTransitions(c1);
    algs.MeasureComboPowers(c1);

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

int main()
{
    ALG_TEST_INIT(ctx, algs);

    ctx.AddNetlist(TEST_NETLIST_DIR "/basic_gates.cdl");

    test_inv        (ctx, algs);

    return 0;
}