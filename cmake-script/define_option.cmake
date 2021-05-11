######################################
# personal properties for -D Option
######################################

set(PROBABILITY "DOUBLE_PROB")

if(LONG_PROBABILITY)
  set(PROBABILITY "LONGDOUBLE_PROB")
ENDIF(LONG_PROBABILITY)

if(QUAD_PROBABILITY)
  set(PROBABILITY "QUAD_PROB")
ENDIF(QUAD_PROBABILITY)

IF(LONG_COSTS)
  SET(COST LONGLONG_COST)
ELSE(LONG_COSTS)
  SET(COST INT_COST)
ENDIF(LONG_COSTS)

IF(SHORT_COSTS)
  SET(COST SHORT_COST)
ENDIF(SHORT_COSTS)

IF(SHORT_VALUES)
  SET(VALUE SHORT_VALUE)
ENDIF(SHORT_VALUES)

IF(BINARYWCSP)
  SET(BINARYWCSPONLY NO_STORE_BINARY_COSTS)
ENDIF(BINARYWCSP)

IF(TERNARYWCSP)
  SET(TERNARYWCSPONLY NO_STORE_TERNARY_COSTS)
ENDIF(TERNARYWCSP)

  IF(TOULBAR2)
    set_property(
      TARGET toulbar2
      PROPERTY COMPILE_DEFINITIONS ${COST} ${VALUE} ${BINARYWCSPONLY} ${TERNARYWCSPONLY} ${XMLFLAG} ${boostflag} ${mpiflag} ${PROBABILITY})
    set_property(
      TARGET tb2-archive
      PROPERTY COMPILE_DEFINITIONS ${COST} ${VALUE} ${BINARYWCSPONLY} ${TERNARYWCSPONLY} ${XMLFLAG} ${boostflag} ${mpiflag} ${PROBABILITY})
    set_property(
      TARGET tb2-objects
      PROPERTY COMPILE_DEFINITIONS ${COST} ${VALUE} ${BINARYWCSPONLY} ${TERNARYWCSPONLY} ${XMLFLAG} ${boostflag} ${mpiflag} ${PROBABILITY})
  ENDIF(TOULBAR2)
  
  IF(MENDELSOFT)
    set_property(
      TARGET mendelsoft
      PROPERTY COMPILE_DEFINITIONS ${COST} ${VALUE} ${BINARYWCSPONLY} ${TERNARYWCSPONLY} MENDELSOFT ${boostflag} ${mpiflag} ${PROBABILITY})
  ENDIF(MENDELSOFT)
  
  IF(LIBTB2)
    set_property(
      TARGET tb2-PIC-objects
      PROPERTY COMPILE_DEFINITIONS ${COST} ${VALUE} ${BINARYWCSPONLY} ${TERNARYWCSPONLY} ${XMLFLAG} ${boostflag} ${mpiflag} ${PROBABILITY})
    set_property(
      TARGET tb2
      PROPERTY COMPILE_DEFINITIONS ${COST} ${VALUE} ${BINARYWCSPONLY} ${TERNARYWCSPONLY} ${XMLFLAG} ${boostflag} ${mpiflag} ${PROBABILITY})
    IF(PYTB2)
      set_property(
	TARGET pytb2
	PROPERTY COMPILE_DEFINITIONS ${COST} ${VALUE} ${BINARYWCSPONLY} ${TERNARYWCSPONLY} ${XMLFLAG} ${boostflag} ${mpiflag} ${PROBABILITY})
    ENDIF(PYTB2)
  ENDIF(LIBTB2)
  
  if(ILOG)
    set_property(
      TARGET iloglue
      PROPERTY COMPILE_DEFINITIONS WCSPFORMATONLY INT_COST ILOGLUE IL_STD ${PROBABILITY})
  ENDIF(ILOG)
