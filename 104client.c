#include "cs104_connection.h"
#include "hal_time.h"
#include "hal_thread.h"


#include <getopt.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h> 
#include <string.h>

bool verbose = false;
char* IP = NULL;

void printCP56Time2a(CP56Time2a time) {
    printf("(%02i:%02i:%02i %02i/%02i/%04i)", CP56Time2a_getHour(time),
                             CP56Time2a_getMinute(time),
                             CP56Time2a_getSecond(time),
                             CP56Time2a_getDayOfMonth(time),
                             CP56Time2a_getMonth(time) + 1,
                             CP56Time2a_getYear(time) + 2000);
}

void printCP24Time2a(CP24Time2a time) {
    printf("(%02imin %02isec)",CP24Time2a_getMinute(time),
                             CP24Time2a_getSecond(time));
}

void printDP(DoublePointInformation DP) {
    int DPValue = DoublePointInformation_getValue(DP);
    switch (DPValue) {
    case 0: printf("INDETERMINATE (0)");    break;
    case 1: printf("OFF (1)");              break;
    case 2: printf("ON  (2)");              break;
    case 3: printf("INDETERMINATE (3)");    break;
    }
}

void printSP(SinglePointInformation SP) {
    bool SPValue = SinglePointInformation_getValue(SP);
    if (SPValue) 
        printf("ON  (1)");
    else 
        printf("OFF (0)");
}

/* Callback handler to log sent or received messages (optional) */ 
static void
rawMessageHandler (void* parameter, uint8_t* msg, int msgSize, bool sent)
{
    if (sent)
        printf("SEND: ");
    else
        printf("RCVD: ");

    int i;
    for (i = 0; i < msgSize; i++) {
        printf("%02x ", msg[i]);
    }

    printf("\n");
}

/* Connection event handler */
static void
connectionHandler (void* parameter, CS104_Connection connection, CS104_ConnectionEvent event)
{
    switch (event) {
    case CS104_CONNECTION_OPENED:
        printf("Connection established\n");
        break;
    case CS104_CONNECTION_CLOSED:
        printf("Connection closed\n");
        break;
    case CS104_CONNECTION_STARTDT_CON_RECEIVED:
        printf("Received STARTDT_CON\n");
        break;
    case CS104_CONNECTION_STOPDT_CON_RECEIVED:
        printf("Received STOPDT_CON\n");
        break;
    }
}

/*
 * CS101_ASDUReceivedHandler implementation
 *
 * For CS104 the address parameter has to be ignored
 */
static bool
asduReceivedHandler (void* parameter, int address, CS101_ASDU asdu)
{
    printf("RECVD ASDU type: %s(%i) elements: %i\n",
            TypeID_toString(CS101_ASDU_getTypeID(asdu)),
            CS101_ASDU_getTypeID(asdu),
            CS101_ASDU_getNumberOfElements(asdu));
    

    if (CS101_ASDU_getTypeID(asdu) == M_ME_NB_1) {
        if (verbose) printf("  measured scaled values:\n");
        int i;
        for (i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
            MeasuredValueScaled io = (MeasuredValueScaled) CS101_ASDU_getElement(asdu, i);
            printf("    CA: %i IOA: %i scaled: %i\n", 
                CS101_ASDU_getCA(asdu),
                InformationObject_getObjectAddress((InformationObject) io), 
                MeasuredValueScaled_getValue((MeasuredValueScaled) io));
            MeasuredValueScaled_destroy(io);
        }
    }
    else if (CS101_ASDU_getTypeID(asdu) == M_ME_TB_1) {
        if (verbose) printf("  measured scaled values with CP24Time2a timestamp:\n");
        int i;
        for (i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
            MeasuredValueScaledWithCP24Time2a io = (MeasuredValueScaledWithCP24Time2a) CS101_ASDU_getElement(asdu, i);
            printf("    CA: %i IOA: %i scaled: %i   ", 
                CS101_ASDU_getCA(asdu),
                InformationObject_getObjectAddress((InformationObject) io), 
                MeasuredValueScaled_getValue((MeasuredValueScaled) io));
            printCP24Time2a(MeasuredValueScaledWithCP24Time2a_getTimestamp(io)); printf("\n");
            MeasuredValueScaledWithCP24Time2a_destroy(io);
        }
    }
    else if (CS101_ASDU_getTypeID(asdu) == M_ME_NC_1) {
        if (verbose) printf("  measured float values:\n");
        int i;
        for (i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
            MeasuredValueShort io = (MeasuredValueShort) CS101_ASDU_getElement(asdu, i);
            printf("    CA: %i IOA: %i float : %f\n",
                CS101_ASDU_getCA(asdu),
                InformationObject_getObjectAddress((InformationObject) io),
                MeasuredValueShort_getValue((MeasuredValueShort) io));
            MeasuredValueShort_destroy(io);
        }
    } 
    else if (CS101_ASDU_getTypeID(asdu) == M_ME_TC_1) {
        if (verbose) printf("  measured float values with CP24Time2a timestamp:\n");
        int i;
        for (i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
            MeasuredValueShortWithCP24Time2a io = (MeasuredValueShortWithCP24Time2a) CS101_ASDU_getElement(asdu, i);
            printf("    CA: %i IOA: %i float : %f   ",
                CS101_ASDU_getCA(asdu),
                InformationObject_getObjectAddress((InformationObject) io),
                MeasuredValueShort_getValue((MeasuredValueShort) io));
            printCP24Time2a(MeasuredValueShortWithCP24Time2a_getTimestamp(io)); printf("\n");
            MeasuredValueShortWithCP24Time2a_destroy(io);
        }
    } 
    else if (CS101_ASDU_getTypeID(asdu) == M_ME_TF_1) {
        if (verbose) printf("  measured float values with CP56Time2a timestamp:\n");
        int i;
        for (i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
            MeasuredValueShortWithCP56Time2a io = (MeasuredValueShortWithCP56Time2a) CS101_ASDU_getElement(asdu, i);
            printf("    CA: %i IOA: %i float : %f   ",
                CS101_ASDU_getCA(asdu),
                InformationObject_getObjectAddress((InformationObject) io),
                MeasuredValueShort_getValue((MeasuredValueShort) io));
            printCP56Time2a(MeasuredValueShortWithCP56Time2a_getTimestamp(io)); printf("\n");
            MeasuredValueShortWithCP56Time2a_destroy(io);
        }
    } 
    else if (CS101_ASDU_getTypeID(asdu) == M_ME_NA_1) {
        if (verbose) printf("  measured normalized values:\n");
        int i;
        for (i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
            MeasuredValueNormalized io = (MeasuredValueNormalized) CS101_ASDU_getElement(asdu, i);
            printf("    CA: %i IOA: %i normal: %f\n",
                CS101_ASDU_getCA(asdu),
                InformationObject_getObjectAddress((InformationObject) io),
                MeasuredValueNormalized_getValue((MeasuredValueNormalized) io));
            MeasuredValueNormalized_destroy(io);
        }
    }           
    else if (CS101_ASDU_getTypeID(asdu) == M_SP_TB_1) {
        if (verbose) printf("  single point information with CP56Time2a timestamp:\n");
        int i;
        for (i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
            SinglePointWithCP56Time2a io = (SinglePointWithCP56Time2a) CS101_ASDU_getElement(asdu, i);
            printf("    CA: %i IOA: %i SPstat: %i    ",
                CS101_ASDU_getCA(asdu),
                InformationObject_getObjectAddress((InformationObject) io),
                SinglePointInformation_getValue((SinglePointInformation) io));
            printCP56Time2a(SinglePointWithCP56Time2a_getTimestamp(io)); printf("\n");
            SinglePointWithCP56Time2a_destroy(io);
        }
    }    
    else if (CS101_ASDU_getTypeID(asdu) == M_DP_NA_1) {
        if (verbose) printf("  double point information:\n");
        int i;
        for (i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
            DoublePointInformation io = (DoublePointInformation) CS101_ASDU_getElement(asdu, i);
            printf("    CA: %i IOA: %i DPstat: ",
                CS101_ASDU_getCA(asdu),
                InformationObject_getObjectAddress((InformationObject) io));
            printDP((DoublePointInformation) io); printf("\n");
            DoublePointInformation_destroy(io);
        }
    }
    else if (CS101_ASDU_getTypeID(asdu) == M_DP_TB_1) {
        if (verbose) printf("  double point information with CP56Time2a timestamp:\n");
        int i;
        for (i = 0; i < CS101_ASDU_getNumberOfElements(asdu); i++) {
            DoublePointWithCP56Time2a io = (DoublePointWithCP56Time2a) CS101_ASDU_getElement(asdu, i);
            printf("    CA: %i IOA: %i DPstat: ",
                CS101_ASDU_getCA(asdu),
                InformationObject_getObjectAddress((InformationObject) io));
            printDP((DoublePointInformation) io); printf("   ");
            printCP56Time2a(DoublePointWithCP56Time2a_getTimestamp(io)); printf("\n");
            DoublePointWithCP56Time2a_destroy(io);
        }
    }
    else if (CS101_ASDU_getTypeID(asdu) == C_IC_NA_1) {
        if (verbose) printf("  Received (General-) Interrogation command\n");
    }
    return true;
}

char* executable_name;

void showHelpAndExit()
{
    printf("Usage: %s -i(IP-Address)\n", executable_name); 
    printf("Optional parameters:\n");
    printf("    -c   -   CASDU adress           (integer)\n");
    printf("    -a   -   IOA                    (integer) \n");
    printf("    -T   -   Connection duration    (integer)\n");
    printf("    -t   -   Command  with type     (integer)\n");
    printf("             Supported types: 45 46 \n");
    printf("    -d   -   Command data           (integer)\n");
    printf("    -g   -   General Interrogation \n");
    printf("    -v   -   Verbose\n");
    printf("    -h   -   Show all IO types \n");
    exit(-1);
}

void showAllTypesAndExit()
{
    printf("Process information in monitoring direction :\n");
    printf("\n");
    printf("1   Single point information                                                    -       M_SP_NA_1\n");
    printf("2   Single point information with time tag                                      -       M_SP_TA_1\n");
    printf("3   Double point information                                                    -       M_DP_NA_1\n");
    printf("4   Double point information with time tag                                      -       M_DP_TA_1\n");
    printf("5   Step position information                                                   -       M_ST_NA_1\n");
    printf("6   Step position information with time tag                                     -       M_ST_TA_1\n");
    printf("7   Bit string of 32 bit                                                        -       M_BO_NA_1\n");
    printf("8   Bit string of 32 bit with time tag                                          -       M_BO_TA_1\n");
    printf("9   Measured value, normalized value                                            -       M_ME_NA_1\n");
    printf("10  Measured value, normalized value with time tag                              -       M_ME_TA_1\n");
    printf("11  Measured value, scaled value                                                -       M_ME_NB_1\n");
    printf("12  Measured value, scaled value with time tag                                  -       M_ME_TB_1\n");
    printf("13  Measured value, short floating point value                                  -       M_ME_NC_1\n");
    printf("14  Measured value, short floating point value with time tag                    -       M_ME_TC_1\n");
    printf("15  Integrated totals                                                           -       M_IT_NA_1\n");
    printf("16  Integrated totals with time tag                                             -       M_IT_TA_1\n");
    printf("17  Event of protection equipment with time tag                                 -       M_EP_TA_1\n");
    printf("18  Packed start events of protection equipment with time tag                   -       M_EP_TB_1\n");
    printf("19  Packed output circuit information of protection equipment with time tag     -       M_EP_TC_1\n");
    printf("20  Packed single-point information with status change detection                -       M_PS_NA_1\n");
    printf("21  Measured value, normalized value without quality descriptor                 -       M_ME_ND_1\n");
    printf("-------------------------------------------------------------------------------------------------\n");
    printf("Process telegrams with long time tag ( 7 octets ) :\n");
    printf("\n");
    printf("30  Single point information with time tag CP56Time2a                           -       M_SP_TB_1\n");
    printf("31  Double point information with time tag CP56Time2a                           -       M_DP_TB_1\n");
    printf("32  Step position information with time tag CP56Time2a                          -       M_ST_TB_1\n");
    printf("33  Bit string of 32 bit with time tag CP56Time2a                               -       M_BO_TB_1\n");
    printf("34  Measured value, normalized value with time tag CP56Time2a                   -       M_ME_TD_1\n");
    printf("35  Measured value, scaled value with time tag CP56Time2a                       -       M_ME_TE_1\n");
    printf("36  Measured value, short floating point value with time tag CP56Time2a         -       M_ME_TF_1\n");
    printf("37  Integrated totals with time tag CP56Time2a                                  -       M_IT_TB_1\n");
    printf("38  Event of protection equipment with time tag CP56Time2a                      -       M_EP_TD_1\n");
    printf("39  Packed start events of protection equipment with time tag CP56time2a        -       M_EP_TE_1\n");
    printf("40  Packed output circuit information of protection equipment with CP56time2a   -       M_EP_TF_1\n");
    printf("-------------------------------------------------------------------------------------------------\n");
    printf("Process information in control direction :\n");
    printf("\n");
    printf("45  Single command                                                              -       C_SC_NA_1\n");
    printf("46  Double command                                                              -       C_DC_NA_1\n");
    printf("47  Regulating step command                                                     -       C_RC_NA_1\n");
    printf("48  Setpoint command, normalized value                                          -       C_SE_NA_1\n");
    printf("49  Setpoint command, scaled value                                              -       C_SE_NB_1\n");
    printf("50  Setpoint command, short floating point value                                -       C_SE_NC_1\n");
    printf("51  Bit string  32 bit                                                          -       C_BO_NA_1\n");
    printf("-------------------------------------------------------------------------------------------------\n");
    printf("Command telegrams with long time tag ( 7 octets ) :                                                         -       \n");
    printf("\n");
    printf("58  Single command with time tag CP56Time2a                                     -       C_SC_TA_1\n");
    printf("59  Double command with time tag CP56Time2a                                     -       C_DC_TA_1\n");
    printf("60  Regulating step command with time tag CP56Time2a                            -       C_RC_TA_1\n");
    printf("61  Setpoint command, normalized value with time tag CP56Time2a                 -       C_SE_TA_1\n");
    printf("62  Setpoint command, scaled value with time tag CP56Time2a                     -       C_SE_TB_1\n");
    printf("63  Setpoint command, short floating point value with time tag CP56Time2a       -       C_SE_TC_1\n");
    printf("64  Bit string 32 bit with time tag CP56Time2a                                  -       C_BO_TA_1\n");
    printf("-------------------------------------------------------------------------------------------------\n");
    printf("System information  in monitoring direction :                                                           -       \n");
    printf("\n");
    printf("70  End of initialization                                                       -       M_EI_NA_1\n");
    printf("-------------------------------------------------------------------------------------------------\n");
    printf("System information in control direction :                                                           -       \n");
    printf("\n");
    printf("100 (General-) Interrogation command                                            -       C_IC_NA_1\n");
    printf("101 Counter interrogation command                                               -       C_CI_NA_1\n");
    printf("102 Read command                                                                -       C_RD_NA_1\n");
    printf("103 Clock synchronization command                                               -       C_CS_NA_1\n");
    printf("104 ( IEC 101 ) Test command                                                    -       C_TS_NB_1\n");
    printf("105 Reset process command                                                       -       C_RP_NC_1\n");
    printf("106 ( IEC 101 ) Delay acquisition command                                       -       C_CD_NA_1\n");
    printf("107 Test command with time tag CP56Time2a                                       -       C_TS_TA_1\n");
    printf("-------------------------------------------------------------------------------------------------\n");
    printf("Parameter in control direction :\n");
    printf("\n");
    printf("110 Parameter of measured value, normalized value                               -       P_ME_NA_1\n");
    printf("111 Parameter of measured value, scaled value                                   -       P_ME_NB_1\n");
    printf("112 Parameter of measured value, short floating point value                     -       P_ME_NC_1\n");
    printf("113 Parameter activation                                                        -       P_AC_NA_1\n");
    printf("-------------------------------------------------------------------------------------------------\n");
    printf("File transfer : \n");
    printf("\n");
    printf("120 File ready                                                                  -       F_FR_NA_1\n");
    printf("121 Section ready                                                               -       F_SR_NA_1\n");
    printf("122 Call directory, select file, call file, call section                        -       F_SC_NA_1\n");
    printf("123 Last section, last segment                                                  -       F_LS_NA_1\n");
    printf("124 Ack file, Ack section                                                       -       F_AF_NA_1\n");
    printf("125 Segment                                                                     -       F_SG_NA_1\n");
    printf("126 Directory                                                                   -       F_DR_TA_1\n");
    printf("127 QueryLog – Request archive file                                             -       F_SC_NB_1\n");

    exit(-1);
}

int
main(int argc, char** argv)
{
  executable_name = argv[0];
  
  if (argc < 2)  showHelpAndExit();


  char* IP = NULL;
  int CA = -1;
  int option;
  int con_secs = 1;
  int ioa = 1;
  TypeID command_type = C_SC_NA_1;
  bool send_command = false;
  bool send_interrogation = false;
  int command_data = 1;

  opterr = 0;


  while ((option = getopt(argc, argv, ":i:c::hvgT::a::t::d::")) != -1)
    {
    switch (option)
      {
      case 'i':
        IP = optarg;
        break;
      case 'c':
        CA = strtol(optarg, NULL, 0);
        break;
      case 'T':
        con_secs = strtol(optarg, NULL, 0);
        break;      
      case 'a':
        ioa = strtol(optarg, NULL, 0);
        break; 
      case 't':
        send_command = true;
        command_type = (TypeID) strtol(optarg, NULL, 0);
        break;
      case 'd':
        send_command = true;
        command_data = strtol(optarg, NULL, 0);
        break;
      case 'g':
        send_interrogation = true;
        break; 
      case 'h':
        showAllTypesAndExit();
        return 1;
      case 'v':
        verbose = true;
        break;
      case '?':
        if (isprint (optopt)) fprintf (stderr, "Unknown option `-%c'.\n\n", optopt);
        showHelpAndExit();
        return 1;
        }
    }

    if (verbose)    printf("Connecting to: %s:%i\n", IP, IEC_60870_5_104_DEFAULT_PORT);
    CS104_Connection con = CS104_Connection_create(IP, IEC_60870_5_104_DEFAULT_PORT);

    CS104_Connection_setConnectionHandler(con, connectionHandler, NULL);


    //CS101_AppLayerParameters alParams = CS104_Connection_getAppLayerParameters(con);
    //alParams->originatorAddress = 3;

    if (verbose) CS104_Connection_setRawMessageHandler(con, rawMessageHandler, NULL);

    if (CS104_Connection_connect(con)) {
        printf("----- IEC 60870-5-104 Connection started ----- \n");

        CS104_Connection_setASDUReceivedHandler(con, asduReceivedHandler, NULL);
        
        CS104_Connection_sendStartDT(con);
        
        Thread_sleep(con_secs * 1000);
        
        if (send_interrogation) {
            printf("----- IEC 60870-5-104 Send Interrogation ----- \n");
            CS104_Connection_sendInterrogationCommand(con, CS101_COT_ACTIVATION, CA, IEC60870_QOI_STATION);
            Thread_sleep(3000);
        }

        if (send_command) {

            InformationObject command_object = NULL;

            printf("----- IEC 60870-5-104 Send %s Command ----- \n",TypeID_toString(command_type));

            switch (command_type)
              {
              
              case C_SC_NA_1:
                command_object = (InformationObject) SingleCommand_create(NULL, ioa, command_data, false, 3);
                break;
              
              case C_DC_NA_1:
                command_object = (InformationObject) DoubleCommand_create(NULL, ioa, command_data, false, 3);
                break;
              
              default:
                fprintf (stderr, "Command %s not supported.\n", TypeID_toString(command_type));
                return 1;
              }
            
            if (verbose) printf("Send control command %s\n",TypeID_toString(command_type));
            
            CS104_Connection_sendProcessCommand(con, command_type, CS101_COT_ACTIVATION, CA, command_object);

            InformationObject_destroy(command_object);
        }
        
    }
    else
        printf("Connect failed!\n");
    
    CS104_Connection_destroy(con);

    Thread_sleep(1000);
    
    if (verbose) printf("exit\n");

}
