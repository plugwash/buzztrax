/* $Id: t-wire.c,v 1.9 2005-09-13 18:51:00 ensonic Exp $
 */

#include "m-bt-core.h"

//-- globals

//-- fixtures

static void test_setup(void) {
  bt_core_setup();
  GST_INFO("================================================================================");
}

static void test_teardown(void) {
	bt_core_teardown();
  //puts(__FILE__":teardown");
}

//-- tests

/**
* try to create a wire with the same machine as source and dest 
*/
START_TEST(test_btwire_obj1){
  BtApplication *app=NULL;
  BtSong *song=NULL;
  BtWire *wire=NULL;
  // machine
  BtProcessorMachine *machine=NULL;
  
  GST_INFO("--------------------------------------------------------------------------------");
  
  /* create a dummy app */
  app=g_object_new(BT_TYPE_APPLICATION,NULL);
  bt_application_new(app);
  
  /* create a new song */
  song=bt_song_new(app);
  fail_unless(song!=NULL,NULL);
  
  /* try to create a source machine */
  machine=bt_processor_machine_new(song,"id","volume",1);
  fail_unless(machine!=NULL,NULL);
  
  check_init_error_trapp("bt_wire_new","src_machine!=dst_machine");
  /* try to add the machine twice to the wire */
  wire=bt_wire_new(song,BT_MACHINE(machine),BT_MACHINE(machine));
  fail_unless(check_has_error_trapped(), NULL);
  fail_unless(wire==NULL,NULL);
}
END_TEST

START_TEST(test_btwire_obj2){
  BtApplication *app=NULL;
  BtSong *song=NULL;
  BtSetup *setup=NULL;
  BtWire *wire1=NULL;
  BtWire *wire2=NULL;
  BtSourceMachine *source=NULL;
  BtProcessorMachine *sink1=NULL;
  BtProcessorMachine *sink2=NULL;
  
  GST_INFO("--------------------------------------------------------------------------------");
  
  /* create a dummy app */
  app=g_object_new(BT_TYPE_APPLICATION,NULL);
  bt_application_new(app);
  
  /* create a new song */
  song=bt_song_new(app);
  fail_unless(song!=NULL,NULL);
  g_object_get(song,"setup",&setup,NULL);
  fail_unless(setup!=NULL, NULL);
 
  /* try to create a source machine */
  source=bt_source_machine_new(song,"id","sinesrc",1);
  fail_unless(source!=NULL,NULL);
  
  /* try to create a volume machine */
  sink1=bt_processor_machine_new(song,"volume1","volume",1);
  fail_unless(sink1!=NULL,NULL);
  
  /* try to create a volume machine */
  sink2=bt_processor_machine_new(song,"volume2","volume",1);
  fail_unless(sink2!=NULL,NULL);

  /* try to connect processor machine to volume1 */
  wire1=bt_wire_new(song,BT_MACHINE(source),BT_MACHINE(sink1));
  mark_point();
  fail_unless(wire1!=NULL,NULL);
  
  /* try to connect processor machine to volume2 */
  wire2=bt_wire_new(song,BT_MACHINE(source),BT_MACHINE(sink2));
  mark_point();
  fail_unless(wire2!=NULL,NULL);

}
END_TEST

TCase *bt_wire_test_case(void) {
  TCase *tc = tcase_create("BtWireTests");

  tcase_add_test(tc,test_btwire_obj1);
  tcase_add_test(tc,test_btwire_obj2);
  tcase_add_unchecked_fixture(tc, test_setup, test_teardown);
  return(tc);
}
