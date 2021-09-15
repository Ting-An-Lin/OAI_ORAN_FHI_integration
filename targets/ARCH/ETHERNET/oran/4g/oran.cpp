/*
 * Licensed to the OpenAirInterface (OAI) Software Alliance under one or more
 * contributor license agreements.  See the NOTICE file distributed with
 * this work for additional information regarding copyright ownership.
 * The OpenAirInterface Software Alliance licenses this file to You under
 * the OAI Public License, Version 1.1  (the "License"); you may not use this file
 * except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.openairinterface.org/?page_id=698
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *-------------------------------------------------------------------------------
 * For more information about the OpenAirInterface (OAI) Software Alliance:
 *      contact@openairinterface.org
 */


#include <stdio.h>
#include "common_lib.h"
// #include "ethernet_lib.h"
#include "oran_south.h"
#include "shared_buffers.h"
#include "low_oran.h"
#include "xran_lib_wrap.hpp"


void xran_fh_rx_callback(void *pCallbackTag, xran_status_t status){
    rte_pause();
}
void xran_fh_srs_callback(void *pCallbackTag, xran_status_t status){
    rte_pause();
}
void xran_fh_rx_prach_callback(void *pCallbackTag, xran_status_t status){
    rte_pause();
}


int trx_oran_start(openair0_device *device)
{
  // xranLibWraper *xranlib = malloc(sizeof(*xranlib)); 
  xranlib = new xranLibWraper;
  
  if(xranlib->SetUp() < 0) {
     return (-1);
  }
  xranlib->Init();
  xranlib->Open(nullptr, 
            nullptr, 
            (void *)xran_fh_rx_callback, 
            (void *)xran_fh_rx_prach_callback, 
            (void *)xran_fh_srs_callback);
  xranlib->Start();
  
  printf("ORAN: %s\n", __FUNCTION__);
  return 0;
}


void trx_oran_end(openair0_device *device)
{
  printf("ORAN: %s\n", __FUNCTION__);
}


int trx_oran_stop(openair0_device *device)
{
  printf("ORAN: %s\n", __FUNCTION__);
  return(0);
}

int trx_oran_set_freq(openair0_device* device,
                         openair0_config_t *openair0_cfg,
                         int exmimo_dump_config)
{
  printf("ORAN: %s\n", __FUNCTION__);
  return(0);
}

int trx_oran_set_gains(openair0_device* device,
                          openair0_config_t *openair0_cfg)
{
  printf("ORAN: %s\n", __FUNCTION__);
  return(0);
}

int trx_oran_get_stats(openair0_device* device)
{
  printf("ORAN: %s\n", __FUNCTION__);
  return(0);
}


int trx_oran_reset_stats(openair0_device* device)
{
  printf("ORAN: %s\n", __FUNCTION__);
  return(0);
}

int ethernet_tune(openair0_device *device,
                  unsigned int option,
                  int value)
{
  printf("ORAN: %s\n", __FUNCTION__);
  return 0;
}

int trx_oran_write_raw(openair0_device *device,
                          openair0_timestamp timestamp,
                          void **buff, int nsamps, int cc, int flags)
{
  printf("ORAN: %s\n", __FUNCTION__);
  return 0;
}

int trx_oran_read_raw(openair0_device *device,
                         openair0_timestamp *timestamp,
                         void **buff, int nsamps, int cc)
{
  printf("ORAN: %s\n", __FUNCTION__);
  return 0;
}

int trx_oran_ctlsend(openair0_device *device, void *msg, ssize_t msg_len)
{

  return 0;
}


int trx_oran_ctlrecv(openair0_device *device, void *msg, ssize_t msg_len)
{

  return 0;
}






// void *get_internal_parameter(char *name)
// {
//   printf("BENETEL: %s\n", __FUNCTION__);

//   if (!strcmp(name, "fh_if4p5_south_in"))
//     return (void *) oran_fh_if4p5_south_in;
//   if (!strcmp(name, "fh_if4p5_south_out"))
//     return (void *) oran_fh_if4p5_south_out;

//   return NULL;
// }

extern "C"
{
  __attribute__((__visibility__("default")))
  int transport_init(openair0_device *device,
                     openair0_config_t *openair0_cfg,
                     eth_params_t * eth_params )
  {
    // oran_eth_state_t *eth;
    printf("Ann: ORANNN\n");
    printf("ORAN: %s\n", __FUNCTION__);

    device->Mod_id               = 0;
    device->transp_type          = ETHERNET_TP;
    device->trx_start_func       = trx_oran_start;
    device->trx_get_stats_func   = trx_oran_get_stats;
    device->trx_reset_stats_func = trx_oran_reset_stats;
    device->trx_end_func         = trx_oran_end;
    device->trx_stop_func        = trx_oran_stop;
    device->trx_set_freq_func    = trx_oran_set_freq;
    device->trx_set_gains_func   = trx_oran_set_gains;

    device->trx_write_func       = trx_oran_write_raw;
    device->trx_read_func        = trx_oran_read_raw;

    device->trx_ctlsend_func     = trx_oran_ctlsend;
    device->trx_ctlrecv_func     = trx_oran_ctlrecv;

    // device->get_internal_parameter = get_internal_parameter;

    init_south(device,
               openair0_cfg,
               eth_params );

    // eth = (oran_eth_state_t *)calloc(1, sizeof(oran_eth_state_t));
    // if (eth == NULL) {
    //   AssertFatal(0==1, "out of memory\n");
    // }

    // eth->e.flags = ETH_RAW_IF4p5_MODE;
    // eth->e.compression = NO_COMPRESS;
    // eth->e.if_name = eth_params->local_if_name;
    // device->priv = eth;
    // device->openair0_cfg=&openair0_cfg[0];

    // eth->last_msg = (rru_config_msg_type_t)-1;

    // init_buffers(&eth->buffers);

    return 0;
  }
}


























