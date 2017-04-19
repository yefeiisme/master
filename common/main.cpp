#include "INetwork.h"
#include "ITabFile.h"

int main(int argc, char *argv[])
{
	g_SetRootPath(".\\");

	IClientNetwork	*m_pClientNetwork = CreateClientNetwork(0, 0, 0, 0, 0, 0, 0, 0);
	if (!m_pClientNetwork)
	{
		// Show Error
		// ...
		return false;
	}
}
