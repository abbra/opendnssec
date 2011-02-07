/*
 * $Id$
 *
 * Copyright (c) 2009 NLNet Labs. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
 * GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
 * IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN
 * IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

/**
 * Zone data.
 *
 */

#ifndef SIGNER_ZONEDATA_H
#define SIGNER_ZONEDATA_H

#include "config.h"
#include "adapter/adapter.h"
#include "shared/allocator.h"
#include "shared/status.h"
#include "signer/denial.h"
#include "signer/domain.h"
#include "signer/keys.h"
#include "signer/signconf.h"
#include "signer/stats.h"
#include "signer/nsec3params.h"

#include <ldns/ldns.h>
#include <stdio.h>


/**
 * Zone data.
 *
 */
typedef struct zonedata_struct zonedata_type;
struct zonedata_struct {
    ldns_rbtree_t* domains;
    ldns_rbtree_t* denial_chain;
    ldns_rbtree_t* nsec3_domains;
    int initialized;
    uint32_t default_ttl; /* fallback ttl */
    uint32_t inbound_serial; /* last seen inbound soa serial */
    uint32_t internal_serial; /* latest internal soa serial */
    uint32_t outbound_serial; /* last written outbound soa serial */
};

/**
 * Create empty zone data.
 * \param[in] allocator memory allocator
 * \return zonedata_type* empty zone data tree
 *
 */
zonedata_type* zonedata_create(allocator_type* allocator);

/**
 * Recover zone data from backup.
 * \param[in] zd zone data
 * \param[in] fd backup file descriptor
 * \return int 0 on success, 1 on error
 *
 */
int zonedata_recover_from_backup(zonedata_type* zd, FILE* fd);

/**
 * Look up domain in zone data.
 * \param[in] zd zone data
 * \param[in] name domain name to look for
 * \return domain_type* domain, if found
 *
 */
domain_type* zonedata_lookup_domain(zonedata_type* zd, ldns_rdf* name);

/**
 * Add domain to zone data.
 * \param[in] zd zone data
 * \param[in] domain domain to add
 * \return domain_type* added domain
 *
 */
domain_type* zonedata_add_domain(zonedata_type* zd, domain_type* domain);

/**
 * Delete domain from zone data.
 * \param[in] zd zone data
 * \param[in] domain domain to delete
 * \return domain_type* domain if failed
 *
 */
domain_type* zonedata_del_domain(zonedata_type* zd, domain_type* domain);

/**
 * Look up denial of existence data point.
 * \param[in] zd zone data
 * \param[in] name domain name to look for
 * \return domain_type* domain, if found
 *
 */
denial_type* zonedata_lookup_denial(zonedata_type* zd, ldns_rdf* name);

/**
 * Add denial of existence data point to zone data.
 * \param[in] zd zone data
 * \param[in] domain corresponding domain
 * \param[in] apex apex
 * \param[in] nsec3params NSEC3 parameters
 * \return ods_status status
 *
 */
ods_status zonedata_add_denial(zonedata_type* zd, domain_type* domain,
    ldns_rdf* apex, nsec3params_type* nsec3params);

/**
 * Delete denial of existence data point from zone data.
 * \param[in] zd zone data
 * \param[in] denial denial of existence data point
 * \return denial_type* denial of existence data point if failed
 *
 */
denial_type* zonedata_del_denial(zonedata_type* zd, denial_type* denial);

/**
 * Examine updates to zone data.
 * \param[in] zd zone data
 * \param[in] apex apex domain name
 * \param[in] mode adapter mode
 * \return ods_status status
 *
 */
ods_status zonedata_examine(zonedata_type* zd, ldns_rdf* apex,
    adapter_mode mode);

/**
 * Commit updates to zone data.
 * \param[in] zd zone data
 * \return ods_status status
 *
 */
ods_status zonedata_commit(zonedata_type* zd);

/**
 * Rollback updates from zone data.
 * \param[in] zd zone data
 *
 */
void zonedata_rollback(zonedata_type* zd);

/**
 * Add empty non-terminals to zone data.
 * \param[in] zd zone data
 * \param[in] apex zone apex
 * \return ods_status status
 *
 */
ods_status zonedata_entize(zonedata_type* zd, ldns_rdf* apex);

/**
 * Add NSEC records to zone data.
 * \param[in] zd zone data
 * \param[in] klass zone class
 * \param[out] stats update statistics
 * \return ods_status status
 *
 */
ods_status zonedata_nsecify(zonedata_type* zd, ldns_rr_class klass,
    stats_type* stats);

/**
 * Add NSEC3 records to zone data.
 * \param[in] zd zone data
 * \param[in] klass zone class
 * \param[in] nsec3params NSEC3 parameters
 * \param[out] stats update statistics
 * \return ods_status status
 *
 */
ods_status zonedata_nsecify3(zonedata_type* zd, ldns_rr_class klass,
    nsec3params_type* nsec3params, stats_type* stats);

/**
 * Add RRSIG records to zone data.
 * \param[in] zd zone data
 * \param[in] owner zone owner
 * \param[in] sc signer configuration
 * \param[out] stats update statistics
 * \return int 0 on success, 1 on false
 *
 */
int zonedata_sign(zonedata_type* zd, ldns_rdf* owner, signconf_type* sc,
    stats_type* stats);

/**
 * Calculate differences at the zonedata between current and new RRsets.
 * \param[in] zd zone data
 * \param[in] kl current key list
 * \return ods_status status
 *
 */
ods_status zonedata_diff(zonedata_type* zd, keylist_type* kl);

/**
 * Commit updates to zone data.
 * \param[in] zd zone data
 * \return ods_status status
 *
 */
ods_status zonedata_commit(zonedata_type* zd);

/**
 * Rollback updates from zone data.
 * \param[in] zd zone data
 *
 */
void zonedata_rollback(zonedata_type* zd);

/**
 * Add RR to zone data.
 * \param[in] zd zone data
 * \param[in] rr RR to add
 * \param[in] at_apex if is at apex of the zone
 * \return int 0 on success, 1 on false
 *
 */
int zonedata_add_rr(zonedata_type* zd, ldns_rr* rr, int at_apex);

/**
 * Recover RR from backup.
 * \param[in] zd zone data
 * \param[in] rr RR to add
 * \return int 0 on success, 1 on false
 *
 */
int zonedata_recover_rr_from_backup(zonedata_type* zd, ldns_rr* rr);

/**
 * Recover RRSIG from backup.
 * \param[in] zd zone data
 * \param[in] rrsig RRSIG to add
 * \param[in] locator key locaotor
 * \param[in] flags key flags
 * \return int 0 on success, 1 on false
 *
 */
int zonedata_recover_rrsig_from_backup(zonedata_type* zd, ldns_rr* rrsig,
    const char* locator, uint32_t flags);

/**
 * Delete RR from zone data.
 * \param[in] zd zone data
 * \param[in] rr RR to delete
 * \return int 0 on success, 1 on false
 *
 */
int zonedata_del_rr(zonedata_type* zd, ldns_rr* rr);

/**
 * Clean up domains in zone data tree.
 * \param[in] domain_tree tree of domains to cleanup
 *
 */
void zonedata_cleanup_domains(ldns_rbtree_t* domain_tree);

/**
 * Wipe out all NSEC RRsets.
 * \param[in] zd zone data
 *
 */
void zonedata_wipe_nsec(zonedata_type* zd);

/**
 * Wipe out NSEC3 tree.
 * \param[in] zd zone data
 *
 */
void zonedata_wipe_nsec3(zonedata_type* zd);

/**
 * Clean up zone data.
 * \param[in] zd zone data to cleanup
 *
 */
void zonedata_cleanup(zonedata_type* zd);

/**
 * Print zone data.
 * \param[in] fd output file descriptor
 * \param[in] zd zone data
 * \return ods_status status
 *
 */
ods_status zonedata_print(FILE* fd, zonedata_type* zd);

/**
 * Print NSEC(3)s in zone data.
 * \param[in] out file descriptor
 * \param[in] zd zone data to print
 *
 */
void zonedata_print_nsec(FILE* fd, zonedata_type* zd);

/**
 * Print RRSIGs in zone data.
 * \param[in] out file descriptor
 * \param[in] zd zone data to print
 *
 */
void zonedata_print_rrsig(FILE* fd, zonedata_type* zd);

#endif /* SIGNER_ZONEDATA_H */
