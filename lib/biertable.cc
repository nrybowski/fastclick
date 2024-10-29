/*
 * biertable.{cc,hh} -- element encapsulates packet in IP6 SRv6 header
 * Nicolas Rybowski
 *
 * Copyright (c) 2024 UCLouvain
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, subject to the conditions
 * listed in the Click LICENSE file. These conditions include: you must
 * preserve this copyright notice, and you cannot mention the copyright
 * holders in advertising related to the Software without their permission.
 * The Software is provided WITHOUT ANY WARRANTY, EXPRESS OR IMPLIED. This
 * notice is a summary of the Click LICENSE file; the license in that file is
 * legally binding.
 */

#include <click/config.h>
#include <click/biertable.hh>
#include <click/glue.hh>
#include <click/straccum.hh>
#include <iostream>
CLICK_DECLS

BierTable::BierTable(){}
BierTable::~BierTable(){}

bool BierTable::lookup(const bfrid &dst, IP6Address &bfr_prefix, bitstring &fbm, IP6Address &nxt, int &index, String &ifname) const {
  int best = -1;

  for (int i = 0; i < _v.size(); i++) {
    if (_v[i]._valid && dst == _v[i]._dst) {
        best = i;
        break;
    }
  }

  if (best < 0)
    return false;
  else {
    nxt = _v[best]._nxt;
    bfr_prefix = _v[best]._bfr_prefix;
    fbm = _v[best]._fbm;
    index = _v[best]._if_idx;
    ifname = _v[best]._if_name;
    return true;
  }
}

void BierTable::del(const bfrid &dst) {
  for (int i = 0; i < _v.size(); i++) {
    if (_v[i]._valid && _v[i]._dst == dst)
      _v[i]._valid = 0;
  }
}

void BierTable::add(const bfrid &dst, const IP6Address bfr_prefix, const bitstring fbm, const IP6Address &nxt, int output, String ifname) {
  struct Entry e;
  e._dst = dst;
  e._bfr_prefix = bfr_prefix;
  e._fbm = fbm;
  e._nxt = nxt;
  e._valid = 1;
  e._if_idx = output;
  e._if_name = ifname;

  // Avoid duplicate routes
  del(dst);
  
  for (int i = 0; i < _v.size(); i++)
    if (!_v[i]._valid) {
      _v[i] = e;
      return;
    }
  _v.push_back(e);
}

String BierTable::dump() {
  StringAccum sa;
  if (_v.size())
    sa << "# Active routes\n# BFR-id   BFR-prefix     F-BM    Nexthop    Ifindex\n";
  for (int i=0; i< _v.size(); i++) {
    if (_v[i]._valid) {
      sa << "    " << _v[i]._dst;
      sa << "    " << _v[i]._bfr_prefix;
      sa << "      " << _v[i]._fbm.unparse();
      sa << "      " << _v[i]._nxt;
      sa << "      " << _v[i]._if_idx;
      sa << "      " << _v[i]._if_name << '\n';
    }
  }
  return sa.take_string();
}

CLICK_ENDDECLS
