#ifndef GUARD_DNS_UTILS_H
#define GUARD_DNS_UTILS_H

#if 0 //DNS
#define DNS_PAL_EXCEPTION   FALSE
#define DNS_PAL_ACTIVE      TRUE

struct LightingColour {
    u8 paletteNum;
    u8 colourNum;
    u16 lightColour;
};

struct DnsPalExceptions {
    bool8 pal[32];
};

void DnsTransferPlttBuffer(void *src, void *dest);
void DnsApplyFilters();
u8 GetDnsTimeLapse(u8 hour);

#endif //DNS

#endif /* GUARD_DNS_UTILS_H */