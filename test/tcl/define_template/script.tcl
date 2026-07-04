define_template T1
define_template -type delay T2
define_template -type delay -index_1 {1 2} T3
define_template -type foo -index_1 {1 2} -index_2 {3 4} T4
define_template -type delay -index_1 {1 2 abc} -index_2 {3 4} T5
define_template -type delay -index_1 {1 2 3} -index_2 {3 4 xyz} T6
define_template -type delay -index_1 {1 3 2} -index_2 {4 5 6} T7
define_template -type delay -index_1 {1 2 3} -index_2 {4 5 6} T7
define_template -type power -index_1 {1 2 3} -index_2 {4 5 6} T8
define_template -type constraint -index_1 {1 2 3} -index_2 {4 5 6} T9
define_template -type delay -index_1 {1 2 3} -index_2 {4 5 6} T7

exit
