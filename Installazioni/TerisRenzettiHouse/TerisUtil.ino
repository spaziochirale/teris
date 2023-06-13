/*
  PROGETTO TERIS CONTROL

  Author: Chirale S.r.l.
  
  Questo file contiene funzioni di utilità generale
  
  */
float rawBitToFloat(uint32_t val){
  // Riceve una variabile di 32 bit e restituisce il valore di tipo float corrispondente
  union
  {   
      uint32_t a ;
      float b ;
  } u ;
  u.a = val;
  return u.b;
}
