/*
 *  COPYRIGHT (C) 2000-2021 ZUUT, INC. ALL RIGHTS RESERVED.
 */
#ifndef BIZOBJ_COUNTRY
#define BIZOBJ_COUNTRY

#line 14 "test_app/test_app.moc" 




   static char header_code= "   typically this is code inserted at the beginning of a header file.   Comments are removed.";



#line 17 "/home/sfmontyo/work/zuut/MOC/moc/foo/include/bizobj/country/Country.h" 

#include <memory>
#include <string>
#include <cstdint>
#include <ctime>
#include <vector>
/**
 * <P>
 * Country
 * a Country.
 *    The country code and name ;
 * </P>
 *
 * @version $Id$
 */
class Country : public BizObject  {
public:
private:    
    /**The printableName property - GUI name. */
    private String printableName;
    /**The numcode property - A numeric code. */
    private String numcode;
};

#line 127 "test_app/test_app.moc" 


   static char header_end_code= "   typically this is code inserted at the end of a header file.";


#line 48 "/home/sfmontyo/work/zuut/MOC/moc/foo/include/bizobj/country/Country.h" 

#endif // BIZOBJ_COUNTRY
