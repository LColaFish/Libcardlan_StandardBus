
#include <unistd.h>
#include <string.h>

#include <ph_NxpBuild.h>
#include <ph_Status.h>
#include <phpalI14443p3a.h>
#include <phpalI14443p4.h>
#include <phKeyStore.h>
#include <phalMfc.h>

static const char	*device =	"/dev/spidev0.0";

#define CHIP_KEY_STORE_MAX		85
#define	CHIP_ID_LENGTH_MAX		10
#define MIFARE_C_ID_LENGTH		4
#define MIFARE_C_KEY_LENGTH		6
#define MIFARE_C_KEY_LENGTH_GROUP	12
#define MIFARE_C_BLOCK_MAX		64
#define MIFARE_C_BLOCK_LENGTH		16


static phStatus_t status;
static phbalReg_Stub_DataParams_t balReader;
static phhalHw_Rc663_DataParams_t halReader;
static phpalI14443p3a_Sw_DataParams_t I14443p3a;
static phpalI14443p4_Sw_DataParams_t I14443p4;
static phpalMifare_Sw_DataParams_t palMifare;
static phKeyStore_Rc663_DataParams_t Rc663keyStore;
static phalMfc_Sw_DataParams_t alReader;
static uint8_t bHalBufferReader[0x400];

int init_dev(const char *dev)
{

	/* Initialize the Reader BAL (Bus Abstraction Layer) component */
	status = phbalReg_Stub_Init(&balReader, sizeof(phbalReg_Stub_DataParams_t));
	if(status != PH_ERR_SUCCESS)
	{
		return -1;
	}
	strcpy(balReader.devPath, dev);
	/* Open device file */
	status = phbalReg_OpenPort(&balReader);
	if(status != PH_ERR_SUCCESS)
	{
		printf("open device file failed\n");
		return -1;
	}
	printf("open device file ok\n");
	/* Initialize the Reader HAL (Hardware Abstraction Layer) component */
	status = phhalHw_Rc663_Init(&halReader, sizeof(phhalHw_Rc663_DataParams_t), &balReader, 0, bHalBufferReader, sizeof(bHalBufferReader), bHalBufferReader, sizeof(bHalBufferReader));
	if(status != PH_ERR_SUCCESS)
	{
		return -1;
	}
	/* Set the parameter to use the spi interface */
	halReader.bBalConnectionType = PHHAL_HW_BAL_CONNECTION_SPI;
	printf("hal layout init ok\n");

	/* Initialize the 14443-3A PAL (Protocol Abstraction Layer) component */
	status = phpalI14443p3a_Sw_Init(&I14443p3a, sizeof(phpalI14443p3a_Sw_DataParams_t), &halReader);
	if(status != PH_ERR_SUCCESS)
	{
		return -1;
	}
	printf("pal 14443-3a layout init ok\n");

	/* Initialize the 14443-4 PAL component */
	status = phpalI14443p4_Sw_Init(&I14443p4, sizeof(phpalI14443p4_Sw_DataParams_t), &halReader);
	if(status != PH_ERR_SUCCESS)
	{
		return -1;
	}
	printf("pal 14443-4 layout init ok\n");

	/* Initialize the Mifare PAL component */
	status = phpalMifare_Sw_Init(&palMifare, sizeof(phpalMifare_Sw_DataParams_t), &halReader, &I14443p4);
	if(status != PH_ERR_SUCCESS)
	{
		return -1;
	}
	printf("pal Mifare layout init ok\n");

	/* Initialize the keystore component */
	status = phKeyStore_Rc663_Init(&Rc663keyStore, sizeof(phKeyStore_Rc663_DataParams_t), &halReader);
	if(status != PH_ERR_SUCCESS)
	{
		return -1;
	}
	printf("keystore layout init ok\n");

	status = phalMfc_Sw_Init(&alReader, sizeof(phalMfc_Sw_DataParams_t), &palMifare, NULL/*&Rc663keyStore*/);
	if(status != PH_ERR_SUCCESS)
	{
		return -1;
	}
	printf("al Mifare classic layout init ok\n");

	/* SoftReset the IC.
	 * The SoftReset only resets the RC663 to EEPROM configuration.
	 */
	status = phhalHw_Rc663_Cmd_SoftReset(&halReader);
	if(status != PH_ERR_SUCCESS)
	{
		printf("Soft reset failed, status is 0x%02X\n", status);
		return -1;
	}
	printf("Soft reset ok!\n");

	/* Reset the RF field */
	status = phhalHw_FieldReset(&halReader);
	if(status != PH_ERR_SUCCESS)
	{
		printf("RF reset failed, status is 0x%x\n", status);
		return -1;
	}
	printf("RF rest ok\n");

	/* Apply the type A protocol settings
	 * and activate the RF field. */
	status = phhalHw_ApplyProtocolSettings(&halReader, PHHAL_HW_CARDTYPE_ISO14443A);
	if(status != PH_ERR_SUCCESS)
	{
		printf("Load 14443a protocol failed, status is 0x%02X\n", status);
		return -1;
	}

	printf("Inin all ok!\n");
	return 0;
}
int halt_card(void)
{
	status = phpalI14443p3a_HaltA(&I14443p3a);
	if(status != PH_ERR_SUCCESS)
	{
		printf("Halt card error, status is 0x%02x\n", status);
		return -1;
	}

	printf("Halt card ok\n");
	return 0;
}


int search_card(	uint8_t bUid[CHIP_ID_LENGTH_MAX])
{
	uint8_t bSak[1];
	uint8_t bMoreCardsAvailable;
	uint8_t bLength;
	uint8_t iq;

	/* Activate the communication layer part 3
	 * of the ISO 14443A standard. */
	status = phpalI14443p3a_ActivateCard(&I14443p3a, NULL, 0, bUid, &bLength, bSak, &bMoreCardsAvailable);
	if(status != PH_ERR_SUCCESS)
	{
		printf("No card searched or error, status is 0x%02x\n", status);
		return NULL;
	}

	// Check if there is a Mifare Classic card in the RF field
	if (0x08 != (*bSak & 0x08))
	{
		printf("Not a Mifare classic card, status is 0x%02x\n", status);
		return NULL;
	}

	/* Check if we have a card in the RF field.
	 * If so, check what card it is. */
	printf("Card ID: ");
	for(iq = 0; iq < bLength; iq++)
	{
		printf("0x%02x ", bUid[iq]);
	}
	printf("\n%d more cards founded\n", bMoreCardsAvailable);

	printf("Search card ok\n");
	return 0;
}
int active_card(uint8_t bUid[CHIP_ID_LENGTH_MAX])
{
	uint8_t bSak[1];
	uint8_t *Uid, UidLength;
	uint8_t bMoreCardsAvailable;
	uint8_t bLength;

	status = phpalI14443p3a_ActivateCard(&I14443p3a, Uid, UidLength, bUid, &bLength, bSak, &bMoreCardsAvailable);
	if(status != PH_ERR_SUCCESS)
	{
		printf("Active card failed, status is 0x%02x\n", status);
		return -1;
	}

	if (0x08 != (*bSak & 0x08))
	{
		printf("Not a Mifare classic card, status is 0x%02x\n", status);
		return -1;
	}

	printf("Active card ok, %d more cards founded\n", bMoreCardsAvailable);
	return 0;
}

int rc663_test(void)
{
    int ret = 0;
    uint8_t bUid[CHIP_ID_LENGTH_MAX];
    uint8_t active_bUid[CHIP_ID_LENGTH_MAX];
    ret = init_dev(device);
    if(ret != 0)
    {
        return -1;
    }
    ret = search_card(bUid);
    if(ret != 0)
    {
        return -2;
    }
    memcpy(active_bUid,bUid,CHIP_ID_LENGTH_MAX);
    ret = halt_card();
    if(ret != 0)
    {
        return -3;
    }
    ret = search_card(bUid);
    if(ret != 0)
    {
        return -4;
    }

    ret = active_card(active_bUid);
    if(ret != 0)
    {
        return -5;
    }
    ret = search_card(bUid);
    if(ret != 0)
    {
        return -6;
    }

    return 0;
}
int main(void)
{
    int ret = 0;
    ret = rc663_test();
    if(ret != 0)
    {
        printf("[ %s %d ] ret = %d \n",__FUNCTION__,__LINE__,ret);
        return -1;
    }

    return 0;
}
